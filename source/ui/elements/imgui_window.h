#ifndef IMGUI_WINDOW_H
#define IMGUI_WINDOW_H

#include "../ui_state.h"

class ImGuiWindow {
public:
    virtual void render(UIState* state) = 0;
    bool visible = false;
};

#endif // IMGUI_WINDOW_H