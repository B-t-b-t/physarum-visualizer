#ifndef EVENT_H
#define EVENT_H

#include <chrono>
#include <iostream>
#include <optional>
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
    TEXT_PRESET_APPLY,
    TEXT_PRESET_CREATE,
    TEXT_PRESET_DELETE,
    TEXT_PRESET_EDIT,

    EDIT_TRAIL_MASK_TIME_SLOT,
    TRAIL_MASK_STRENGTH_CHANGED
};

struct TimeSlot {
    std::chrono::system_clock::time_point start;
    std::chrono::system_clock::time_point end;
};

enum class TrailMaskType {
    IMAGE,
    TEXT
};

struct TrailMaskData {
    std::string name{};
    TrailMaskType type{};
    
    std::optional<TimeSlot> timeSlot{std::nullopt};

    std::optional<size_t> atIndex{std::nullopt};  //at which index the change applies
};

using EventPayload = std::variant<
    std::monostate,
    int,
    float,
    std::string,
    TrailMaskData
>; 

struct UserEvent {
    EventType type;

    //two values, e.g. if an event requires an index (data) and what changes at this index (additionalData)
    //or resizing a texture (width and height)
    EventPayload payload{std::monostate{}};
};

std::ostream& operator<<(std::ostream& os, const EventType& c);

#endif // EVENT_H