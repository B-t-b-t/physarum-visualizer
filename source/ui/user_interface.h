#ifndef USER_INTERFACE_H
#define USER_INTERFACE_H

#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

#include "imgui.h"
#include "imgui_impl_opengl3.h"
#include "imgui_impl_sdl3.h"

#include "ui_state.h"
#include "../uniforms.h"
#include "../audio/audio_system.h"
#include "elements/imgui_window.h"

class UserInterface {

public:

	//UserInterface();
	UserInterface(SDL_Window* window, SDL_GLContext glContext);
	~UserInterface();

	void display(std::vector<double>& audioBuffer, std::vector<double>& spectrum, std::vector<double>& spectrumDiff, int bufferSize, bool hasNewSpectrumData);
	ImGuiWindow* getWindow(std::string windowName) {return windows_.at(windowName).get();}
	UIState* getState() { return state_; }

private:

	UIState* state_;
	std::unordered_map<std::string, std::unique_ptr<ImGuiWindow>> windows_;

	ImGuiIO *guiIO_;

	bool showNew_ = false;
	bool showImGUIDemo_ = false;
	bool showImPlotDemo_ = false;

	void initWindows();

	void newModal();
	void mainMenuBarGUI();
};

#endif // USER_INTERFACE_H