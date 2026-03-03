#pragma once
#include "ImGuiWindow.h"
#include "../UserInterface.h"

class VisualSettingsWindow : public ImGuiWindow {

public:
    void render(UIState &state) override;
};