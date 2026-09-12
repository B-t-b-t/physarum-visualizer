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
    EDIT_TRAIL_MASK_TIME_SLOT,
    CREATE_NEW_TEXT_TEXTURE,
    EDIT_TEXT_TEXTURE,
    DELETE_TEXT_TEXTURE,
    TRAIL_MASK_STRENGTH_CHANGED
};

struct TrailMaskData {
    std::string newName;
    bool isText;
    bool hasTimeSlot;

    int dayBegin;
    int hourBegin;
    int minuteBegin;

    int dayEnd;
    int hourEnd;
    int minuteEnd;
};

std::ostream& operator<<(std::ostream& os, const EventType& c);

struct UserEvent {
    EventType type;

    //two values, e.g. if an event requires an index (data) and what changes at this index (additionalData)
    //or resizing a texture (width and height)
    std::variant<int, float, std::string, TrailMaskData> data_1;
    std::variant<int, float, std::string, TrailMaskData> data_2;
};

#endif // EVENT_H