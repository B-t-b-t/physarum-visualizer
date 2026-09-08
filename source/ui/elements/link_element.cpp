#include "link_element.h"

#include <algorithm>         // for copy_n
#include <stddef.h>          // for size_t
#include <typeindex>         // for type_index
#include <vector>            // for vector

#include "imgui.h"           // for ImVec2, IM_ASSERT, ImDrawList, ImGuiStyle
#include "imgui_internal.h"  // for ImGuiWindow, ImGuiWindowTempData, ImRect

namespace {

struct RegisteredLinkValue {
    void* value = nullptr;
    std::type_index type = typeid(void);
    void (*copyValue)(void* destination, const void* source) = nullptr;
};

struct LinkContext {
    bool active = false;
    const char* id = nullptr;
    bool* isLinked = nullptr;
    float thickness = 1.0f;
    float capLength = 5.0f;
    int changedIndex = -1;
    std::vector<RegisteredLinkValue> values;
};

thread_local LinkContext linkContext;

struct ColorEdit3LinkValue {};

void copyColorEdit3Value(void* destination, const void* source) {
    float* destinationColor = static_cast<float*>(destination);
    const float* sourceColor = static_cast<const float*>(source);

    std::copy_n(sourceColor, 3, destinationColor);
}

void registerColorEdit3Value(float* color, bool changed) {
    IM_ASSERT(linkContext.active);
    IM_ASSERT(color != nullptr);

    if (!linkContext.active || color == nullptr) {
        return;
    }

    if (changed) {
        linkContext.changedIndex = static_cast<int>(linkContext.values.size());
    }

    linkContext.values.push_back({
        color,
        typeid(ColorEdit3LinkValue),
        &copyColorEdit3Value,
    });
}

template <typename T>
void copyLinkValue(void* destination, const void* source) {
    *static_cast<T*>(destination) = *static_cast<const T*>(source);
}

template <typename T>
void registerLinkValue(T* value, bool changed) {
    IM_ASSERT(linkContext.active);
    IM_ASSERT(value != nullptr);

    if (!linkContext.active || value == nullptr)
        return;

    if (changed)
        linkContext.changedIndex = static_cast<int>(linkContext.values.size());

    linkContext.values.push_back({
        value,
        typeid(T),
        &copyLinkValue<T>,
    });
}

void resetLinkContext() {
    linkContext.active = false;
    linkContext.id = nullptr;
    linkContext.isLinked = nullptr;
    linkContext.thickness = 1.0f;
    linkContext.capLength = 5.0f;
    linkContext.changedIndex = -1;
    linkContext.values.clear();
}

bool drawLinkButton(
    const char* id,
    bool* isLinked,
    int elementCount,
    float thickness,
    float capLength) {
    ImGuiWindow* window = ImGui::GetCurrentWindow();
    if (window->SkipItems)
        return false;

    IM_ASSERT(id != nullptr);
    IM_ASSERT(isLinked != nullptr);
    IM_ASSERT(elementCount >= 2);

    if (id == nullptr || isLinked == nullptr || elementCount < 2)
        return false;

    thickness = ImMax(thickness, 1.0f);
    capLength = ImMax(capLength, 0.0f);

    //linkEnd() must be called directly after the final linked input
    const ImRect lastInput(ImGui::GetItemRectMin(), ImGui::GetItemRectMax());
    const ImGuiStyle& style = ImGui::GetStyle();

    const float inputHeight = lastInput.GetHeight();
    const float lineLeft = lastInput.Max.x + style.ItemSpacing.x;
    const float lineBottom = lastInput.Max.y;
    const float lineTop = lastInput.Min.y
        - static_cast<float>(elementCount - 1) * (inputHeight + style.ItemSpacing.y);

    const ImU32 lineColor = ImGui::GetColorU32(ImGuiCol_Separator);
    const float lineCenterX = lineLeft + thickness * 0.5f;

    window->DrawList->AddRectFilled(
        ImVec2(lineLeft, lineTop),
        ImVec2(lineLeft + thickness, lineBottom),
        lineColor);

    //draw left-facing caps, creating a bracket around the linked inputs
    window->DrawList->AddLine(
        ImVec2(lineCenterX, lineTop),
        ImVec2(lineCenterX - capLength, lineTop),
        lineColor,
        thickness);

    window->DrawList->AddLine(
        ImVec2(lineCenterX, lineBottom),
        ImVec2(lineCenterX - capLength, lineBottom),
        lineColor,
        thickness);

    const float buttonHeight = ImGui::GetTextLineHeight();
    const float buttonY = (lineTop + lineBottom - buttonHeight) * 0.5f;

    //the button is an overlay and must not affect the normal ImGui layout
    const ImVec2 savedCursorPos = window->DC.CursorPos;
    const ImVec2 savedCursorPosPrevLine = window->DC.CursorPosPrevLine;
    const ImVec2 savedCurrLineSize = window->DC.CurrLineSize;
    const ImVec2 savedPrevLineSize = window->DC.PrevLineSize;
    const float savedCurrLineTextBaseOffset = window->DC.CurrLineTextBaseOffset;
    const float savedPrevLineTextBaseOffset = window->DC.PrevLineTextBaseOffset;
    const bool savedIsSameLine = window->DC.IsSameLine;

    ImGui::SetCursorScreenPos(ImVec2(
        lineLeft + thickness + style.ItemSpacing.x,
        buttonY));

    ImGui::PushID(id);
    const bool clicked = ImGui::SmallButton(*isLinked ? "" : "");
    ImGui::SetItemTooltip(*isLinked ? "Linked" : "Unlinked");
    ImGui::PopID();

    if (clicked)
        *isLinked = !*isLinked;

    window->DC.CursorPos = savedCursorPos;
    window->DC.CursorPosPrevLine = savedCursorPosPrevLine;
    window->DC.CurrLineSize = savedCurrLineSize;
    window->DC.PrevLineSize = savedPrevLineSize;
    window->DC.CurrLineTextBaseOffset = savedCurrLineTextBaseOffset;
    window->DC.PrevLineTextBaseOffset = savedPrevLineTextBaseOffset;
    window->DC.IsSameLine = savedIsSameLine;

    return clicked;
}

} // end namespace

void ImGui::LinkBegin(const char* id, bool* isLinked, float thickness, float capLength) {
    IM_ASSERT(!linkContext.active);
    IM_ASSERT(id != nullptr);
    IM_ASSERT(isLinked != nullptr);

    if (linkContext.active || id == nullptr || isLinked == nullptr)
        return;

    linkContext.active = true;
    linkContext.id = id;
    linkContext.isLinked = isLinked;
    linkContext.thickness = thickness;
    linkContext.capLength = capLength;
    linkContext.changedIndex = -1;
    linkContext.values.clear();
}

bool ImGui::LinkEnd() {
    IM_ASSERT(linkContext.active);

    if (!linkContext.active)
        return false;

    if (linkContext.values.size() < 2) {
        resetLinkContext();
        return false;
    }

    const bool clicked = drawLinkButton(
        linkContext.id,
        linkContext.isLinked,
        static_cast<int>(linkContext.values.size()),
        linkContext.thickness,
        linkContext.capLength);

    bool matchingTypes = true;
    const std::type_index firstType = linkContext.values.front().type;

    for (const RegisteredLinkValue& value : linkContext.values) {
        if (value.type != firstType || value.value == nullptr || value.copyValue == nullptr) {
            matchingTypes = false;
            break;
        }
    }

    IM_ASSERT(matchingTypes);

    //synchronize only when a control changed, or when the link was enabled
    if (matchingTypes
        && *linkContext.isLinked
        && (clicked || linkContext.changedIndex >= 0)) {
        int sourceIndex = linkContext.changedIndex;

        //enabling the link without editing a control uses the first value
        if (sourceIndex < 0)
            sourceIndex = 0;

        const RegisteredLinkValue& source = linkContext.values[(size_t)sourceIndex];

        for (const RegisteredLinkValue& target : linkContext.values)
            target.copyValue(target.value, source.value);
    }

    resetLinkContext();
    return clicked;
}

bool ImGui::LinkSliderFloat(
    const char* label,
    float* value,
    float min,
    float max,
    const char* format,
    ImGuiSliderFlags flags) 
{
    const bool changed = ImGui::SliderFloat(label, value, min, max, format, flags);
    registerLinkValue(value, changed);
    return changed;
}

bool ImGui::LinkInputFloat(
    const char* label,
    float* value,
    float step,
    float stepFast,
    const char* format,
    ImGuiInputTextFlags flags) 
{
    const bool changed = ImGui::InputFloat(label, value, step, stepFast, format, flags);
    registerLinkValue(value, changed);
    return changed;
}

bool ImGui::LinkSliderInt(
    const char* label,
    int* value,
    int min,
    int max,
    const char* format,
    ImGuiSliderFlags flags) 
{
    const bool changed = ImGui::SliderInt(label, value, min, max, format, flags);
    registerLinkValue(value, changed);
    return changed;
}

bool ImGui::LinkInputInt(
    const char* label,
    int* value,
    int step,
    int stepFast,
    ImGuiInputTextFlags flags) 
{
    const bool changed = ImGui::InputInt(label, value, step, stepFast, flags);
    registerLinkValue(value, changed);
    return changed;
}

bool ImGui::LinkColorEdit3(
    const char* label,
    float color[3],
    ImGuiColorEditFlags flags) 
{
    const bool changed = ImGui::ColorEdit3(label, color, flags);
    registerColorEdit3Value(color, changed);
    return changed;
}