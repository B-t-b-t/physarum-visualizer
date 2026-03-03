#pragma once
#include "../UIState.h"

class ImGuiWindow {
public:
    virtual void render(UIState& state) = 0;
    bool visible = false;
};