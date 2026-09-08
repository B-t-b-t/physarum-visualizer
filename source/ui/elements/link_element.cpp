#include "link_element.h"

#include <imgui.h>           // for ImVec2, Button, GetColorU32, ImDrawList
#include "imgui_internal.h"  // for ImGuiWindow, ImGuiWindowTempData, ImRect

#include "link_element.h"

#include <imgui.h>
#include "imgui_internal.h"

bool ImGui::link(const char* id, bool* isLinked, int elementCount, float thickness, float capLength) {
    ImGuiWindow* window = GetCurrentWindow();
    if (window->SkipItems)
        return false;

    IM_ASSERT(id != nullptr);
    IM_ASSERT(isLinked != nullptr);
    IM_ASSERT(elementCount >= 2);

    if (isLinked == nullptr)
        return false;

    elementCount = ImMax(elementCount, 2);
    capLength = ImMax(capLength, 0.0f);

    //link() must be called directly after the last linked input
    const ImRect lastInput(GetItemRectMin(), GetItemRectMax());
    const ImGuiStyle& style = GetStyle();

    const float inputHeight = lastInput.GetHeight();
    const float lineLeft = lastInput.Max.x + style.ItemSpacing.x;
    const float lineBottom = lastInput.Max.y;
    const float lineTop = lastInput.Min.y
        - static_cast<float>(elementCount - 1) * (inputHeight + style.ItemSpacing.y);

    const ImU32 lineColor = GetColorU32(ImGuiCol_Separator);
    const float lineCenterX = lineLeft + thickness * 0.5f;

    window->DrawList->AddRectFilled(
        ImVec2(lineLeft, lineTop),
        ImVec2(lineLeft + thickness, lineBottom),
        lineColor);

    //draw left facing caps so the line forms a bracket around the inputs
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

    //small button has text line height rather than GetFrameHeight()
    const float buttonHeight = GetTextLineHeight();
    const float buttonY = (lineTop + lineBottom - buttonHeight) * 0.5f;

    //save ImGui layout state. The button is an overlay and must not move
    //the cursor used by the widget following this link
    const ImVec2 savedCursorPos = window->DC.CursorPos;
    const ImVec2 savedCursorPosPrevLine = window->DC.CursorPosPrevLine;
    const ImVec2 savedCurrLineSize = window->DC.CurrLineSize;
    const ImVec2 savedPrevLineSize = window->DC.PrevLineSize;
    const float savedCurrLineTextBaseOffset = window->DC.CurrLineTextBaseOffset;
    const float savedPrevLineTextBaseOffset = window->DC.PrevLineTextBaseOffset;
    const bool savedIsSameLine = window->DC.IsSameLine;

    SetCursorScreenPos(ImVec2(
        lineLeft + thickness + style.ItemSpacing.x,
        buttonY));

    PushID(id);
    const bool clicked = SmallButton(*isLinked ? "" : "");
    ImGui::SetItemTooltip(*isLinked ? "Linked" : "Unlinked");
    PopID();

    if (clicked)
        *isLinked = !*isLinked;

    //restore the cursor so subsequent widgets remain below the last input
    window->DC.CursorPos = savedCursorPos;
    window->DC.CursorPosPrevLine = savedCursorPosPrevLine;
    window->DC.CurrLineSize = savedCurrLineSize;
    window->DC.PrevLineSize = savedPrevLineSize;
    window->DC.CurrLineTextBaseOffset = savedCurrLineTextBaseOffset;
    window->DC.PrevLineTextBaseOffset = savedPrevLineTextBaseOffset;
    window->DC.IsSameLine = savedIsSameLine;

    return clicked;
}