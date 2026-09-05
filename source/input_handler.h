#ifndef INPUT_HANDLER_H
#define INPUT_HANDLER_H

class ApplicationState; // Forward declaration of ApplicationState class
#include "SDL3/SDL_video.h"   // for SDL_Window

class InputHandler {

public:
    InputHandler(ApplicationState* appState);

    void processUserInput(SDL_Window* window);

private:

    ApplicationState* appState_;
};

#endif // INPUT_HANDLER_H