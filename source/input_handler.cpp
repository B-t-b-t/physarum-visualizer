#include "input_handler.h"

#include <iostream>            // for basic_ostream, char_traits, cout, endl

#include "imgui_impl_sdl3.h"   // for ImGui_ImplSDL3_ProcessEvent
#include "SDL3/SDL_events.h"   // for SDL_EventType, SDL_Event, SDL_EventAction
#include "SDL3/SDL_keycode.h"  // for SDLK_ESCAPE, SDLK_F11

class ApplicationState; // Forward declaration of ApplicationState class

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