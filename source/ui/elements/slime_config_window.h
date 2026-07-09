#ifndef SLIME_CONFIG_WINDOW_H
#define SLIME_CONFIG_WINDOW_H

#include "imgui_window.h"
#include "../user_interface.h"

class SlimeConfigWindow : public ImGuiWindow {

public:
    void render(ApplicationState* appState) override;
};

#endif // SLIME_CONFIG_WINDOW_H