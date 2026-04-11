#ifndef DEBUG_WINDOW_H
#define DEBUG_WINDOW_H

#include "imgui_window.h"
#include "../user_interface.h"

class DebugWindow : public ImGuiWindow {

public:
    void render(UIState &state) override;

    void setGuiIO(ImGuiIO *guiIO) { guiIO_ = guiIO; };

private:

    ImGuiIO *guiIO_;
};

#endif // DEBUG_WINDOW_H