#pragma once
#include "imgui.h"
#include "imgui_impl_sdl3.h"
#include "imgui_impl_opengl3.h"
#include <string>
#include <vector>
#include <memory>
#include <unordered_map>
#include "../audio/AudioRecording.h"
#include "../Uniforms.h"
#include "elements/ImGuiWindow.h"
#include "UIState.h"

class UserInterface {

public:

	UserInterface(SDL_Window* window, SDL_GLContext glContext);
	~UserInterface();

	void display(std::vector<double>& audioBuffer, std::vector<double>& spectrum, std::vector<double>& spectrumDiff, int bufferSize, bool hasNewSpectrumData);
	ImGuiWindow* getWindow(std::string windowName) {return windows_.at(windowName).get();}
	UIState& getState() { return state_; }

private:

	UIState& state_;
	std::unordered_map<std::string, std::unique_ptr<ImGuiWindow>> windows_;

	ImGuiIO *guiIO_;

	bool showNew_ = false;
	bool showImGUIDemo_ = false;
	bool showImPlotDemo_ = false;

	void initWindows();

	void newModal();
	void mainMenuBarGUI();
};