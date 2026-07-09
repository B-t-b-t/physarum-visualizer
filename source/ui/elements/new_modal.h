#ifndef NEW_MODAL_H
#define NEW_MODAL_H

#include "imgui_window.h"

class NewModal : public ImGuiWindow {

public:
    void render(ApplicationState* appState) override;
};

#endif // NEW_MODAL_H