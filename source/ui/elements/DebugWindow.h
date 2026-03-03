#pragma once
#include "ImGuiWindow.h"
#include "../UserInterface.h"

class DebugWindow : public ImGuiWindow {

public:
    void render(UIState &state) override;

    void setGuiIO(ImGuiIO& guiIO) { guiIO_ = guiIO; };

private:

    ImGuiIO guiIO_;
};