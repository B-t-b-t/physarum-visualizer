#ifndef EVENT_H
#define EVENT_H

#include <iostream>

enum class Event {
    WINDOW_RESIZE,
    TEXTURE_RESIZE,
    FULLSCREEN_TOGGLE,
    NEW_CANVAS,
    AUDIO_HARDWARE_CHANGE,
    SAVE_PRESET,
    LOAD_PRESET,
    SAVE_COLOR_PRESET,
    LOAD_COLOR_PRESET,
    LOAD_NEW_PICTURE
};

std::ostream& operator<<(std::ostream& os, const Event& c);

#endif // EVENT_H