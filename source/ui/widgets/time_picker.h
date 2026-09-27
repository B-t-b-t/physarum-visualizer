#ifndef TIME_PICKER_H
#define TIME_PICKER_H

#include <chrono>

namespace ImGui {
    bool timePicker(const char* ID, std::chrono::system_clock::time_point* time);
    bool timePicker(const char* ID, std::chrono::hh_mm_ss<std::chrono::seconds>* time);
}

#endif // TIME_PICKER_H