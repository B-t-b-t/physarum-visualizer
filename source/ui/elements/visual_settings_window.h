#ifndef VISUAL_SETTINGS_WINDOW_H
#define VISUAL_SETTINGS_WINDOW_H

#include "imgui_window.h"
#include "../user_interface.h"

class VisualSettingsWindow : public ImGuiWindow {

public:
    void render(UIState* state) override;
};

#endif // VISUAL_SETTINGS_WINDOW_H