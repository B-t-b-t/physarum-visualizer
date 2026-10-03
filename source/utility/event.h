#ifndef EVENT_H
#define EVENT_H

#include <stddef.h>  // for size_t
#include <chrono>    // for system_clock
#include <cstdint>   // for uint32_t
#include <iostream>  // for ostream
#include <optional>  // for optional, nullopt, nullopt_t
#include <string>    // for basic_string, string
#include <variant>   // for monostate, variant

#include "time_handling.h"  // for TimeSlot
#include "vector_math.h"  // for phys::Vec2
#include "../simulation/trail_mask.h"  // for TrailMaskData

enum class EventType {
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
    TEXT_PRESET_EDIT
};

struct NewCanvasData {
    int newTextureWidth{};
    int newTextureHeight{};
    int newNumParticles{};
};

using EventPayload = std::variant<
    std::monostate,
    int,
    float,
    uint32_t,   //for AUDIO_HARDWARE_CHANGE using SDL_AudioDeviceID (uint32_t)
    std::string,
    TrailMask,  //only allow TrailMask and not TrailMaskProperties, because the receiver additionally needs to know name and type to know which TrailMask is being referred to
    NewCanvasData
>; 

struct UserEvent {
    EventType type;

    //two values, e.g. if an event requires an index (data) and what changes at this index (additionalData)
    //or resizing a texture (width and height)
    EventPayload payload{std::monostate{}};
};

std::ostream& operator<<(std::ostream& os, const EventType& c);

#endif // EVENT_H