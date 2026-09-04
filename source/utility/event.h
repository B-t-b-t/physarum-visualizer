#ifndef EVENT_H
#define EVENT_H

#include <iostream>
#include <string>
#include <variant>

enum class EventType {
    WINDOW_RESIZE,
    TEXTURE_RESIZE,
    FULLSCREEN_TOGGLE,
    NEW_CANVAS,
    AUDIO_HARDWARE_CHANGE,
    SAVE_PRESET,
    LOAD_PRESET,
    SAVE_COLOR_PRESET,
    LOAD_COLOR_PRESET,
    LOAD_NEW_PICTURE,
    CREATE_NEW_TEXT_TEXTURE,
    EDIT_TEXT_TEXTURE,
    DELETE_TEXT_TEXTURE
};

std::ostream& operator<<(std::ostream& os, const EventType& c);

struct UserEvent {
    EventType type;
    std::variant<int, float, std::string> data;
};

#endif // EVENT_H