#include "user_interface.h"

#include "ui_helpers.h"
#include "elements/audio_window.h"
#include "elements/debug_window.h"
#include "elements/preset_window.h"
#include "elements/slime_config_window.h"
#include "elements/visual_settings_window.h"

UserInterface::UserInterface(SDL_Window* window, SDL_GLContext glContext) : state_(UIState::getInstance()) {
		
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

	(dynamic_cast<DebugWindow*>(windows_.at("DebugWindow").get()))->setGuiIO(guiIO_);
}

void UserInterface::display(std::vector<double>& audioBuffer, std::vector<double>& spectrum, std::vector<double>& spectrumDiff, int bufferSize, bool hasNewSpectrumData) {
	// Start the Dear ImGui frame
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplSDL3_NewFrame();
	ImGui::NewFrame();

	if(!guiIO_->WantCaptureMouse) {
		state_.universalShaderSettings.mouseInputs = ImVec4(guiIO_->MousePos.x, guiIO_->MousePos.y, ImGui::IsMouseDown(0), ImGui::IsMouseDown(1));
	}

	mainMenuBarGUI();
	if(showNew_) {newModal();}

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
}

void UserInterface::newModal() {
	if (showNew_) {
		ImGui::OpenPopup("New Canvas");
	}

	// Always center this window when appearing
	ImVec2 center = ImGui::GetMainViewport()->GetCenter();
	ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));

	if (ImGui::BeginPopupModal("New Canvas", NULL, ImGuiWindowFlags_AlwaysAutoResize)) {

		ImGui::Text("Create a new Canvas?\nThis operation cannot be undone!");
		ImGui::Separator();


		ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0, 0));
		ImGui::InputInt("New Texture Width", &state_.newTextureWidth, 8, 8, ImGuiInputTextFlags_CharsNoBlank);
		state_.newTextureWidth = state_.newTextureWidth - (state_.newTextureWidth % 8);
		ImGui::InputInt("New Texture Height", &state_.newTextureHeight, 8, 8, ImGuiInputTextFlags_CharsNoBlank);
		state_.newTextureHeight = state_.newTextureHeight - (state_.newTextureHeight % 8);
		state_.newNumParticles = state_.slimeRatio * state_.newTextureWidth * state_.newTextureHeight;
		ImGui::InputInt("Number of Particles", &state_.newNumParticles, 8, 8, ImGuiInputTextFlags_CharsNoBlank);
		state_.newNumParticles = state_.newNumParticles - (state_.newNumParticles % 8);
		state_.slimeRatio = state_.newNumParticles / (float) (state_.newTextureWidth * state_.newTextureHeight);
		ImGui::InputFloat("Slime Ratio", &state_.slimeRatio);
		ImGui::PopStyleVar();
		ImGui::Separator();

		ImGui::SliderFloat("v", &state_.slimeSettings.v, 0.0f, 3.0f);
		ImGui::SliderInt("Rotation Angle", &state_.slimeSettings.rotationAngle, 0, 180);
		ImGui::SliderInt("Sensor Angle", &state_.slimeSettings.angle, 0, 180);
		
		ImGui::SliderInt("Sensor Distance", &state_.slimeSettings.sensorDistance, 1, 100);
		ImGui::SliderFloat("Deposition Strength", &state_.slimeSettings.depositionStrength, 0.0f, 10.0f);
		ImGui::SliderFloat("diffusionWeight", &state_.trailDiffusionSettings.diffusionWeight, 0.0f, 1.0f);
		ImGui::SliderFloat("decay", &state_.trailDiffusionSettings.decay, 0.0f, 1.0f);

		ImGui::Separator();
		
		ImGui::ColorEdit3("Slime Color 0", (float*)&state_.slimeSettings.slimeColor0);
		ImGui::ColorEdit3("Slime Color 1", (float*)&state_.slimeSettings.slimeColor1);
		ImGui::ColorEdit3("Slime Color 2", (float*)&state_.slimeSettings.slimeColor2);
		
		ImGui::Separator();
		
		ImGui::Checkbox("Use Particle Mask instead of Color", (bool*)&state_.slimeSettings.useMask);
		
		ImGui::Checkbox("Collision Detection", (bool*)&state_.universalShaderSettings.collisionDetection);

		if (ImGui::Button("OK", ImVec2(120, 0))) {  showNew_ = false;
													//state_.universalShaderSettings.textureWidth = state_.newTextureWidth;
													//state_.universalShaderSettings.textureHeight = state_.newTextureHeight;
													//state_.universalShaderSettings.windowWidth = state_.newTextureWidth;
													//state_.universalShaderSettings.windowHeight = state_.newTextureHeight;
													state_.numParticles = state_.newNumParticles;
													state_.newCanvas = true; 
													ImGui::CloseCurrentPopup(); }
		ImGui::SetItemDefaultFocus();
		ImGui::SameLine();
		if (ImGui::Button("Cancel", ImVec2(120, 0))) { showNew_ = false; ImGui::CloseCurrentPopup(); }
		ImGui::EndPopup();
	}
}

void UserInterface::mainMenuBarGUI() {
	ImGuiIO& io = ImGui::GetIO();
	if (io.MousePos.y <= 200 && ImGui::BeginMainMenuBar()) {
        if (ImGui::BeginMenu("File")) {
			if (ImGui::MenuItem("New Canvas")) {showNew_ = true;}
			ImGui::Separator();
            if (ImGui::MenuItem("Exit")) {state_.exitProgram = true;}
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
