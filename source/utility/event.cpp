#include "event.h"

std::ostream& operator<<(std::ostream& os, const Event& c) {
    switch (c) {
        case Event::WINDOW_RESIZE:    return os << "WINDOW_RESIZE";
        case Event::TEXTURE_RESIZE:   return os << "TEXTURE_RESIZE";
        case Event::FULLSCREEN_TOGGLE: return os << "FULLSCREEN_TOGGLE";
        case Event::NEW_CANVAS:       return os << "NEW_CANVAS";
        case Event::AUDIO_HARDWARE_CHANGE: return os << "AUDIO_HARDWARE_CHANGE";
        case Event::SAVE_PRESET:      return os << "SAVE_PRESET";
        case Event::LOAD_PRESET:      return os << "LOAD_PRESET";
        case Event::SAVE_COLOR_PRESET: return os << "SAVE_COLOR_PRESET";
        case Event::LOAD_COLOR_PRESET: return os << "LOAD_COLOR_PRESET";
        case Event::LOAD_NEW_PICTURE: return os << "LOAD_NEW_PICTURE";
        default:                      return os << "<Missing String Representation for Event Enum>";
    }
}