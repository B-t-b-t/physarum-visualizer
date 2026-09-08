#ifndef USER_INTERFACE_H
#define USER_INTERFACE_H

#include <memory>
#include <string>
#include <unordered_map>

#include "imgui.h"
#include "imgui_impl_opengl3.h"
#include "imgui_impl_sdl3.h"

#include "../application_state.h"
#include "../uniforms.h"
#include "../audio/audio_system.h"
#include "elements/base_window.h"

class UserInterface {

public:

	//UserInterface();
	UserInterface(SDL_Window* window, SDL_GLContext glContext, ApplicationState* appState);
	~UserInterface();

	void display();
	BaseWindow* getWindow(std::string windowName) {return windows_.at(windowName).get();}
	ApplicationState* getState() { return state_; }
	bool wantsInput() { return guiIO_->WantCaptureMouse || guiIO_->WantCaptureKeyboard; }

private:

	ApplicationState* state_;
	std::unordered_map<std::string, std::unique_ptr<BaseWindow>> windows_;

	ImGuiIO *guiIO_;

	SDL_Window* window_;
	SDL_GLContext glContext_;

	#if DEBUG
	bool showImGUIDemo_ = false;
	bool showImPlotDemo_ = false;
	#endif

	void initWindows();

	void mainMenuBarGUI();
};

#endif // USER_INTERFACE_H