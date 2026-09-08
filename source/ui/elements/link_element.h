#ifndef LINK_ELEMENT_H
#define LINK_ELEMENT_H

#include "imgui.h"

namespace ImGui {

    // Must be followed by LinkSliderFloat(), LinkInputFloat(), LinkSliderInt(),
    // or LinkInputInt() calls, then LinkEnd().
    void LinkBegin(
        const char* id,
        bool* isLinked,
        float thickness = 1.0f,
        float capLength = 5.0f);

    // Draws the link button and applies a changed linked value to every value in
    // the current LinkBegin()/LinkEnd() scope. Returns true when the button was
    // clicked.
    bool LinkEnd();

    // Wrapper around normal ImGui Float Slider. 
    // Supports value linking with other LinkSliderFloats.
    bool LinkSliderFloat(
        const char* label,
        float* value,
        float min,
        float max,
        const char* format = "%.3f",
        ImGuiSliderFlags flags = 0);

    // Wrapper around normal ImGui Float Input. 
    // Supports value linking with other LinkInputFloats.
    bool LinkInputFloat(
        const char* label,
        float* value,
        float step = 0.0f,
        float stepFast = 0.0f,
        const char* format = "%.3f",
        ImGuiInputTextFlags flags = 0);

    // Wrapper around normal ImGui Int Slider. 
    // Supports value linking with other LinkSliderInts.
    bool LinkSliderInt(
        const char* label,
        int* value,
        int min,
        int max,
        const char* format = "%d",
        ImGuiSliderFlags flags = 0);

    // Wrapper around normal ImGui Int Input. 
    // Supports value linking with other LinkInputInts.
    bool LinkInputInt(
        const char* label,
        int* value,
        int step = 1,
        int stepFast = 100,
        ImGuiInputTextFlags flags = 0);

    bool LinkColorEdit3(
        const char* label,
        float color[3],
        ImGuiColorEditFlags flags = 0);

}

#endif // LINK_ELEMENT_H