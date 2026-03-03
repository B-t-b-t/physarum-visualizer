#pragma once
#include "ImGuiWindow.h"
#include "../UserInterface.h"

class SlimeConfigWindow : public ImGuiWindow {

public:
    void render(UIState &state) override;
};