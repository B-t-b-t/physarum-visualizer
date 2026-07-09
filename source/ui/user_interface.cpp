#include "user_interface.h"

#include "ui_helpers.h"
#include "elements/audio_window.h"
#include "elements/debug_window.h"
#include "elements/new_modal.h"
#include "elements/preset_window.h"
#include "elements/slime_config_window.h"
#include "elements/visual_settings_window.h"

UserInterface::UserInterface(SDL_Window* window, SDL_GLContext glContext, ApplicationState* appState)
 : state_(appState)
 {
		
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImPlot::CreateContext();

	ImGui::StyleColorsDark();

	ImGui_ImplSDL3_InitForOpenGL(window, glContext);
	ImGui_ImplOpenGL3_Init();

	guiIO_ = &ImGui::GetIO();
	guiIO_->ConfigFlags |= ImGuiConfigFlags_DockingEnable;

	initWindows();
}

UserInterface::~UserInterface() {
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplSDL3_Shutdown();
	ImPlot::DestroyContext();
	ImGui::DestroyContext();
}

void UserInterface::initWindows() {
	windows_.emplace("SlimeConfigWindow", std::make_unique<SlimeConfigWindow>());
	windows_.emplace("VisualSettingsWindow", std::make_unique<VisualSettingsWindow>());
	windows_.emplace("PresetWindow", std::make_unique<PresetWindow>());
	windows_.emplace("AudioWindow", std::make_unique<AudioWindow>());
	windows_.emplace("DebugWindow", std::make_unique<DebugWindow>());
	windows_.emplace("NewModal", std::make_unique<NewModal>());

	(dynamic_cast<DebugWindow*>(windows_.at("DebugWindow").get()))->setGuiIO(guiIO_);
}

void UserInterface::display(std::vector<double>& audioBuffer, std::vector<double>& spectrum, std::vector<double>& spectrumDiff, int bufferSize, bool hasNewSpectrumData) {
	// Start the Dear ImGui frame
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplSDL3_NewFrame();
	ImGui::NewFrame();

	if(!guiIO_->WantCaptureMouse) {
		state_->universalShaderSettings.mouseInputs = ImVec4(guiIO_->MousePos.x, guiIO_->MousePos.y, ImGui::IsMouseDown(0), ImGui::IsMouseDown(1));
	}

	mainMenuBarGUI();

	if(showImGUIDemo_) {ImGui::ShowDemoWindow();}
	if(showImPlotDemo_) {ImPlot::ShowDemoWindow();}

	AudioWindow* audioWindow = dynamic_cast<AudioWindow*>(windows_.at("AudioWindow").get());
    if (audioWindow && audioWindow->visible) {
        audioWindow->update(audioBuffer, spectrum, spectrumDiff, bufferSize, hasNewSpectrumData);
    }

	for(auto& kv : windows_) {
		auto& w = kv.second;
		if (w && w->visible) { w->render(state_); }
	}

	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void UserInterface::mainMenuBarGUI() {
	ImGuiIO& io = ImGui::GetIO();
	if (io.MousePos.y <= 200 && ImGui::BeginMainMenuBar()) {
        if (ImGui::BeginMenu("File")) {
			if (ImGui::MenuItem("New Canvas")) {windows_.at("NewModal")->visible = true;}
			ImGui::Separator();
            if (ImGui::MenuItem("Exit")) {state_->exitProgram = true;}
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("View")) {
            if (ImGui::MenuItem("Slime Config", "")) { 
				bool* showSlimeWindow = &(windows_.at("SlimeConfigWindow")->visible);
				*showSlimeWindow = *showSlimeWindow ? false : true;
			}
			if (ImGui::MenuItem("Visual Settings", "")) { 
				bool* showVisualSettingsWindow = &(windows_.at("VisualSettingsWindow")->visible);
				*showVisualSettingsWindow = *showVisualSettingsWindow ? false : true;
			}
			if (ImGui::MenuItem("Preset", "")) {
				bool* showPresetWindow = &(windows_.at("PresetWindow")->visible);
				*showPresetWindow = *showPresetWindow ? false : true;
			}
            if (ImGui::MenuItem("Audio", "")) {
				bool* showAudioWindow = &(windows_.at("AudioWindow")->visible);
				*showAudioWindow = *showAudioWindow ? false : true;
			}
			if (ImGui::MenuItem("Debug", "")) {
				bool* showDebugWindow = &(windows_.at("DebugWindow")->visible);
				*showDebugWindow = *showDebugWindow ? false : true;
			}
			ImGui::Separator();
			if (ImGui::MenuItem("ImGui Demo", "", showImGUIDemo_)) {showImGUIDemo_ = showImGUIDemo_ ? false : true;}
			if (ImGui::MenuItem("ImPlot Demo", "", showImPlotDemo_)) {showImPlotDemo_ = showImPlotDemo_ ? false : true;}
            ImGui::EndMenu();
        }
        ImGui::EndMainMenuBar();
    }
}
