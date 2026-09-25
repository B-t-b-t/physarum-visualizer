#ifndef PRESET_WINDOW_HELPER_H
#define PRESET_WINDOW_HELPER_H

#include <cstddef>

class ApplicationState; // Forward declaration of ApplicationState
class PresetWindow; // Forward declaration of PresetWindow
class TrailMask; // Forward declaration of TrailMask

namespace PresetWindowHelper {
    void behaviorPresetAddModal(const char* stringID, ApplicationState* appState, PresetWindow* presetWindow);
    void behaviorPresetDeleteModal(const char* stringID, ApplicationState* appState, PresetWindow* presetWindow);

    void colorPresetAddModal(const char* stringID, ApplicationState* appState, PresetWindow* presetWindow);
    void colorPresetDeleteModal(const char* stringID, ApplicationState* appState, PresetWindow* presetWindow);

    void imagePresetEditModal(const char* stringID, TrailMask& trailMask, PresetWindow* presetWindow, size_t i);

    void textPresetAddModal(const char* stringID, PresetWindow* presetWindow);
    void textPresetEditModal(const char* stringID, TrailMask& trailMask, PresetWindow* presetWindow, size_t* i);
}

#endif // PRESET_WINDOW_HELPER_H
