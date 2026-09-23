#ifndef COLOR_INDICATOR_H
#define COLOR_INDICATOR_H

#include <initializer_list>

#include "imgui.h"

namespace ImGui {

/**
 * @brief Draws a simple color indicator with multiple colors.
 * 
 * Displays each color with a colored circle, number of circles depends on the number of colors provided.
 * Base size depends on font scale (text height).
 * Lightweight, no mouse interaction, hovering, ...
 * 
 * @param colors A list of colors to display.
 * @param scale The scale of the color indicator (Default: TextHeight).
 * @param borderThickness The thickness of the border around each circle (Default: 1.0f, Disable with 0.0f)
 */
void ColorIndicator(std::initializer_list<const ImVec4> colors, float scale = 1.0f, float borderThickness = 1.0f);

}

#endif // COLOR_INDICATOR_H