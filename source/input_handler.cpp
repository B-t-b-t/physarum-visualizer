#include "input_handler.h"

#include <iostream>            // for basic_ostream, char_traits, cout, endl

#include "imgui_impl_sdl3.h"   // for ImGui_ImplSDL3_ProcessEvent
#include "SDL3/SDL_events.h"   // for SDL_EventType, SDL_Event, SDL_EventAction
#include "SDL3/SDL_keycode.h"  // for SDLK_ESCAPE, SDLK_F11

#include "application_state.h"   // for ApplicationState

class ApplicationState; // Forward declaration of ApplicationState class

InputHandler::InputHandler(ApplicationState* appState)
 : appState_{appState}
{
    
}

void InputHandler::processUserInput(SDL_Window* window) {

    float mouseX = 0.0f;
    float mouseY = 0.0f;
    const SDL_MouseButtonFlags mouseButtons = SDL_GetMouseState(&mouseX, &mouseY);

    int logicalWidth = 0;
    int logicalHeight = 0;
    int pixelWidth = 0;
    int pixelHeight = 0;

    SDL_GetWindowSize(window, &logicalWidth, &logicalHeight);
    SDL_GetWindowSizeInPixels(window, &pixelWidth, &pixelHeight);

    const float logicalToPixelX =
        logicalWidth > 0
            ? static_cast<float>(pixelWidth) / static_cast<float>(logicalWidth)
            : 1.0f;

    const float logicalToPixelY =
        logicalHeight > 0
            ? static_cast<float>(pixelHeight) / static_cast<float>(logicalHeight)
            : 1.0f;

    // mouseX/Y are local to the SDL window. Convert exactly once to the
    // framebuffer-pixel coordinate system used by windowWidth/windowHeight.
    appState_->universalShaderSettings.mouseInputs.x = mouseX * logicalToPixelX;
    appState_->universalShaderSettings.mouseInputs.y = mouseY * logicalToPixelY;
    appState_->universalShaderSettings.mouseInputs.z =
        (mouseButtons & SDL_BUTTON_LMASK) != 0 ? 1.0f : 0.0f;
    appState_->universalShaderSettings.mouseInputs.w =
        (mouseButtons & SDL_BUTTON_RMASK) != 0 ? 1.0f : 0.0f;

    //removed because it breaks input handling in window.cpp and dettached ImGui windows
    /*
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
    */
}