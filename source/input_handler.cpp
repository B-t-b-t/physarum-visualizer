#include "input_handler.h"

#include <iostream>

InputHandler::InputHandler(ApplicationState* appState)
 : appState_{appState}
{
    
}

void InputHandler::processUserInput() {
    SDL_Event inputEvent;

    SDL_PumpEvents(); //necessary to update the event queue with latest events

    //filter just keyboard and mouse events
	while (SDL_PeepEvents(&inputEvent, 1, SDL_GETEVENT, SDL_EVENT_KEY_DOWN, SDL_EVENT_MOUSE_REMOVED)) {
		ImGui_ImplSDL3_ProcessEvent(&inputEvent);

		switch (inputEvent.type) {
            case SDL_EVENT_KEY_DOWN:
                if(inputEvent.key.key == SDLK_ESCAPE) {std::cout << "ESCAPE" << std::endl;}
                if(inputEvent.key.key == SDLK_F11) {std::cout << "F11" << std::endl;}
                break;
            default:
                break;
		}
	}
}