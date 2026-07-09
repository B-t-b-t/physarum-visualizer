#ifndef IMGUI_WINDOW_H
#define IMGUI_WINDOW_H

#include "../../application_state.h"
#include "../../utility/observable.h"

class ImGuiWindow : public Observable {
public:
    virtual void render(ApplicationState* state) = 0;
    bool visible = false;
};

#endif // IMGUI_WINDOW_H