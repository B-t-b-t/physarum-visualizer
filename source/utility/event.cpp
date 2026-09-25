#include "event.h"

std::ostream& operator<<(std::ostream& os, const EventType& c) {
    switch (c) {
        case EventType::WINDOW_RESIZE:    return os << "WINDOW_RESIZE";
        case EventType::TEXTURE_RESIZE:   return os << "TEXTURE_RESIZE";
        case EventType::FULLSCREEN_TOGGLE: return os << "FULLSCREEN_TOGGLE";
        case EventType::NEW_CANVAS:       return os << "NEW_CANVAS";
        case EventType::AUDIO_HARDWARE_CHANGE: return os << "AUDIO_HARDWARE_CHANGE";
        case EventType::BEHAVIOR_PRESET_CREATE:      return os << "BEHAVIOR_PRESET_CREATE";
        case EventType::BEHAVIOR_PRESET_APPLY:      return os << "BEHAVIOR_PRESET_APPLY";
        case EventType::BEHAVIOR_PRESET_DELETE:      return os << "BEHAVIOR_PRESET_DELETE";
        case EventType::COLOR_PRESET_CREATE: return os << "COLOR_PRESET_CREATE";
        case EventType::COLOR_PRESET_APPLY: return os << "COLOR_PRESET_APPLY";
        case EventType::COLOR_PRESET_DELETE: return os << "COLOR_PRESET_DELETE";
        case EventType::IMAGE_PRESET_APPLY: return os << "IMAGE_PRESET_APPLY";
        case EventType::EDIT_TRAIL_MASK_TIME_SLOT: return os << "EDIT_TRAIL_MASK_TIME_SLOT";
        case EventType::TEXT_PRESET_CREATE: return os << "TEXT_PRESET_CREATE";
        case EventType::TEXT_PRESET_EDIT: return os << "TEXT_PRESET_EDIT";
        case EventType::TEXT_PRESET_DELETE: return os << "TEXT_PRESET_DELETE";
        case EventType::TRAIL_MASK_STRENGTH_CHANGED: return os << "TRAIL_MASK_STRENGTH_CHANGED";
        default:                      return os << "<Missing String Representation for Event Enum>";
    }
}