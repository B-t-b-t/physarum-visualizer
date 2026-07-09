#ifndef NEW_CANVAS_MODAL_H
#define NEW_CANVAS_MODAL_H

#include "imgui_window.h"

class NewCanvasModal : public ImGuiWindow {

public:
    void render(ApplicationState* appState) override;
};

#endif // NEW_CANVAS_MODAL_H