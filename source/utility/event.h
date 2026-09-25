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

    BEHAVIOR_PRESET_APPLY,
    BEHAVIOR_PRESET_CREATE,
    BEHAVIOR_PRESET_DELETE,
    BEHAVIOR_PRESET_EDIT,
    COLOR_PRESET_APPLY,
    COLOR_PRESET_CREATE,
    COLOR_PRESET_DELETE,
    COLOR_PRESET_EDIT,
    IMAGE_PRESET_APPLY,
    IMAGE_PRESET_CREATE,
    IMAGE_PRESET_DELETE,
    IMAGE_PRESET_EDIT,
    TEXT_PRESET_CREATE,
    TEXT_PRESET_DELETE,
    TEXT_PRESET_EDIT,

    EDIT_TRAIL_MASK_TIME_SLOT,
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