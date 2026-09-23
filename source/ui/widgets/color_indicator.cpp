#define IMGUI_DEFINE_MATH_OPERATORS

#include "color_indicator.h"

#include "imgui_internal.h"


void ImGui::ColorIndicator(std::initializer_list<const ImVec4> colors, float scale, float borderThickness)
{
    ImGuiWindow* window = GetCurrentWindow();

    //early return when window invisible
    if (window->SkipItems) {
        return;
    }

    ImDrawList* drawList = ImGui::GetWindowDrawList();
    const float textHeight = GetTextLineHeight();   //identical to font scale
    const float circleRadius = (textHeight / 2) * scale;
    const float borderCircleRadius = circleRadius + borderThickness / 2;    //border shouldn't overlap with filled circle, so divide by 2
    const float boundingSize = (circleRadius + borderThickness) * 2;    //absolute size of one color indicator

    ImVec2 pos = window->DC.CursorPos;

    //draw indicator for each color
    for(auto col : colors) {
        drawList->AddCircleFilled(ImVec2(pos.x + circleRadius, pos.y + (textHeight / 2)), circleRadius, GetColorU32(col));

        //border
        if (borderThickness > 0.0f) {
            drawList->AddCircle(ImVec2(pos.x + circleRadius, pos.y + (textHeight / 2)), borderCircleRadius, GetColorU32(ImGuiCol_Border), 0, borderThickness);
        }

        pos.x += boundingSize + textHeight / 8;   //gap between circles scales with text size
    }
    
    NewLine();  //set cursor to next line

    return;
}