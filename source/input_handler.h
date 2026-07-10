#ifndef INPUT_HANDLER_H
#define INPUT_HANDLER_H

#include "imgui_impl_sdl3.h"
#include <SDL3/SDL.h>

#include "application_state.h"

class InputHandler {

public:
    InputHandler(ApplicationState* appState);

    void processUserInput();

private:

    ApplicationState* appState_;
};

#endif // INPUT_HANDLER_H