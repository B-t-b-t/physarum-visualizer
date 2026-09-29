#ifndef TIME_PICKER_H
#define TIME_PICKER_H

#include <chrono>

namespace ImGui {

    /**
     * @brief Displays a time picker widget for selecting hours, minutes, and seconds.
     * 
     * This time picker displays the provided timepoint as three buttons for hours, minutes, and seconds.
     * When clicked, a popup list will visually extend the clicked button, 
     * allowing the user to select the corresponding hour, minute, or second.
     * This will only change the daytime component of the timepoint.
     * 
     * @param ID ImGui ID to uniquely identify the widget.
     * @param time Pointer to a time_point, whose daytime component can be changed by this time picker.
     * @return true if the time was changed.
     */
    bool timePicker(const char* ID, std::chrono::system_clock::time_point* time);

    /**
     * @brief Displays a date picker widget for selecting a date.
     * 
     * This date picker displays the provided timepoint as three buttons for day, month, and year.
     * When clicked, a popup list will visually extend the clicked button, 
     * allowing the user to select the corresponding day, month or year. 
     * This will only change the date component of the timepoint.
     * 
     * @param ID ImGui ID to uniquely identify the widget.
     * @param time Pointer to a time_point, whose date component can be changed by this date picker.
     * @return True if the date was changed.
     */
    bool datePicker(const char* ID, std::chrono::system_clock::time_point* time);
}

#endif // TIME_PICKER_H