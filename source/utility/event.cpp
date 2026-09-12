#include "event.h"

std::ostream& operator<<(std::ostream& os, const EventType& c) {
    switch (c) {
        case EventType::WINDOW_RESIZE:    return os << "WINDOW_RESIZE";
        case EventType::TEXTURE_RESIZE:   return os << "TEXTURE_RESIZE";
        case EventType::FULLSCREEN_TOGGLE: return os << "FULLSCREEN_TOGGLE";
        case EventType::NEW_CANVAS:       return os << "NEW_CANVAS";
        case EventType::AUDIO_HARDWARE_CHANGE: return os << "AUDIO_HARDWARE_CHANGE";
        case EventType::SAVE_PRESET:      return os << "SAVE_PRESET";
        case EventType::LOAD_PRESET:      return os << "LOAD_PRESET";
        case EventType::SAVE_COLOR_PRESET: return os << "SAVE_COLOR_PRESET";
        case EventType::LOAD_COLOR_PRESET: return os << "LOAD_COLOR_PRESET";
        case EventType::LOAD_NEW_PICTURE: return os << "LOAD_NEW_PICTURE";
        case EventType::EDIT_TRAIL_MASK_TIME_SLOT: return os << "EDIT_TRAIL_MASK_TIME_SLOT";
        case EventType::CREATE_NEW_TEXT_TEXTURE: return os << "CREATE_NEW_TEXT_TEXTURE";
        case EventType::EDIT_TEXT_TEXTURE: return os << "EDIT_TEXT_TEXTURE";
        case EventType::DELETE_TEXT_TEXTURE: return os << "DELETE_TEXT_TEXTURE";
        case EventType::TRAIL_MASK_STRENGTH_CHANGED: return os << "TRAIL_MASK_STRENGTH_CHANGED";
        default:                      return os << "<Missing String Representation for Event Enum>";
    }
}