#include "user_interface.h"

#include "ui_helpers.h"
#include "elements/audio_window.h"
#include "elements/debug_window.h"
#include "elements/new_canvas_modal.h"
#include "elements/preset_window.h"
#include "elements/slime_config_window.h"
#include "elements/visual_settings_window.h"

inline void ImGui_StyleNuklearDarkGray();

UserInterface::UserInterface(SDL_Window* window, SDL_GLContext glContext, ApplicationState* appState)
 : state_(appState)
 {
		
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImPlot::CreateContext();

	//ImGui::StyleColorsDark();
	ImGui_StyleNuklearDarkGray();

	ImGui_ImplSDL3_InitForOpenGL(window, glContext);
	ImGui_ImplOpenGL3_Init();

	guiIO_ = &ImGui::GetIO();
	guiIO_->ConfigFlags |= ImGuiConfigFlags_DockingEnable;
	guiIO_->Fonts->AddFontFromFileTTF("res/fonts/Roboto-Medium.ttf", 16.0f);

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
	windows_.emplace("NewCanvasModal", std::make_unique<NewCanvasModal>());

	(dynamic_cast<DebugWindow*>(windows_.at("DebugWindow").get()))->setGuiIO(guiIO_);
}

void UserInterface::display() {
	// Start the Dear ImGui frame
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplSDL3_NewFrame();
	ImGui::NewFrame();

	if(!guiIO_->WantCaptureMouse) {
		state_->universalShaderSettings.mouseInputs = ImVec4(guiIO_->MousePos.x * state_->fractionalScalingFactor, guiIO_->MousePos.y * state_->fractionalScalingFactor, ImGui::IsMouseDown(0), ImGui::IsMouseDown(1));
	}

	mainMenuBarGUI();

	if(showImGUIDemo_) {ImGui::ShowDemoWindow();}
	if(showImPlotDemo_) {ImPlot::ShowDemoWindow();}

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
			if (ImGui::MenuItem("New Canvas")) {windows_.at("NewCanvasModal")->visible = true;}
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
			
			#if _DEBUG
			ImGui::Separator();
			if (ImGui::MenuItem("ImGui Demo", "", showImGUIDemo_)) {showImGUIDemo_ = showImGUIDemo_ ? false : true;}
			if (ImGui::MenuItem("ImPlot Demo", "", showImPlotDemo_)) {showImPlotDemo_ = showImPlotDemo_ ? false : true;}
			#endif

            ImGui::EndMenu();
        }
        ImGui::EndMainMenuBar();
    }
}


//by Zalthen-dev from here: https://github.com/ocornut/imgui/issues/707#issuecomment-4763488457
inline void ImGui_StyleNuklearDarkGray() {
	ImGuiStyle* style = &ImGui::GetStyle();
	ImVec4* colors = style->Colors;

	//style->FontScaleDpi = 2.0f;

	style->WindowBorderSize = 1.0f;
	style->ChildBorderSize = 1.0f;
	style->PopupBorderSize = 1.0f;
	style->FrameBorderSize = 1.0f;

	style->WindowRounding = 10.0f;
	style->ChildRounding = 10.0f;
	style->FrameRounding = 4.0f;
	style->PopupRounding = 4.0f;
	style->GrabRounding = 2.0f;

	colors[ImGuiCol_Text]                   = ImVec4(0.69f, 0.69f, 0.69f, 1.00f);
	colors[ImGuiCol_TextDisabled]           = ImVec4(0.35f, 0.35f, 0.35f, 1.00f);
	colors[ImGuiCol_WindowBg]               = ImVec4(0.18f, 0.18f, 0.18f, 1.00f);
	colors[ImGuiCol_ChildBg]                = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
	colors[ImGuiCol_PopupBg]                = ImVec4(0.18f, 0.18f, 0.18f, 1.00f);
	colors[ImGuiCol_Border]                 = ImVec4(0.22f, 0.22f, 0.22f, 1.00f);
	colors[ImGuiCol_BorderShadow]           = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
	colors[ImGuiCol_FrameBg]                = ImVec4(0.15f, 0.15f, 0.15f, 1.00f);
	colors[ImGuiCol_FrameBgHovered]         = ImVec4(0.29f, 0.29f, 0.29f, 1.00f);
	colors[ImGuiCol_FrameBgActive]          = ImVec4(0.39f, 0.39f, 0.39f, 1.00f);
	colors[ImGuiCol_TitleBg]                = ImVec4(0.15f, 0.15f, 0.15f, 1.00f);
	colors[ImGuiCol_TitleBgActive]          = ImVec4(0.18f, 0.18f, 0.18f, 1.00f);
	colors[ImGuiCol_TitleBgCollapsed]       = ImVec4(0.15f, 0.15f, 0.15f, 1.00f);
	colors[ImGuiCol_MenuBarBg]              = ImVec4(0.14f, 0.14f, 0.14f, 1.00f);
	colors[ImGuiCol_ScrollbarBg]            = ImVec4(0.18f, 0.18f, 0.18f, 1.00f);
	colors[ImGuiCol_ScrollbarGrab]          = ImVec4(0.31f, 0.31f, 0.31f, 1.00f);
	colors[ImGuiCol_ScrollbarGrabHovered]   = ImVec4(0.41f, 0.41f, 0.41f, 1.00f);
	colors[ImGuiCol_ScrollbarGrabActive]    = ImVec4(0.51f, 0.51f, 0.51f, 1.00f);
	colors[ImGuiCol_CheckMark]              = ImVec4(0.69f, 0.69f, 0.69f, 1.00f);
	colors[ImGuiCol_CheckboxSelectedBg]     = ImVec4(0.15f, 0.15f, 0.15f, 0.50f);
	colors[ImGuiCol_SliderGrab]             = ImVec4(0.40f, 0.40f, 0.40f, 1.00f);
	colors[ImGuiCol_SliderGrabActive]       = ImVec4(0.59f, 0.59f, 0.59f, 1.00f);
	colors[ImGuiCol_Button]                 = ImVec4(0.22f, 0.22f, 0.22f, 1.00f);
	colors[ImGuiCol_ButtonHovered]          = ImVec4(0.15f, 0.15f, 0.15f, 1.00f);
	colors[ImGuiCol_ButtonActive]           = ImVec4(0.12f, 0.12f, 0.12f, 1.00f);
	colors[ImGuiCol_Header]                 = ImVec4(0.22f, 0.22f, 0.22f, 0.78f);
	colors[ImGuiCol_HeaderHovered]          = ImVec4(0.22f, 0.22f, 0.22f, 0.78f);
	colors[ImGuiCol_HeaderActive]           = ImVec4(0.29f, 0.29f, 0.29f, 0.78f);
	colors[ImGuiCol_Separator]              = ImVec4(0.29f, 0.29f, 0.29f, 0.50f);
	colors[ImGuiCol_SeparatorHovered]       = ImVec4(0.49f, 0.49f, 0.49f, 0.78f);
	colors[ImGuiCol_SeparatorActive]        = ImVec4(0.69f, 0.69f, 0.69f, 1.00f);
	colors[ImGuiCol_ResizeGrip]             = ImVec4(0.29f, 0.29f, 0.29f, 1.00f);
	colors[ImGuiCol_ResizeGripHovered]      = ImVec4(0.49f, 0.49f, 0.49f, 1.00f);
	colors[ImGuiCol_ResizeGripActive]       = ImVec4(0.69f, 0.69f, 0.69f, 1.00f);
	colors[ImGuiCol_InputTextCursor]        = ImVec4(0.78f, 0.78f, 0.78f, 1.00f);
	colors[ImGuiCol_TabHovered]             = ImVec4(0.49f, 0.49f, 0.49f, 0.80f);
	colors[ImGuiCol_Tab]                    = ImVec4(0.29f, 0.29f, 0.29f, 1.00f);
	colors[ImGuiCol_TabSelected]            = ImVec4(0.39f, 0.39f, 0.39f, 1.00f);
	colors[ImGuiCol_TabSelectedOverline]    = ImVec4(0.15f, 0.15f, 0.15f, 1.00f);
	colors[ImGuiCol_TabDimmed]              = ImVec4(0.29f, 0.29f, 0.29f, 0.78f);
	colors[ImGuiCol_TabDimmedSelected]      = ImVec4(0.39f, 0.39f, 0.39f, 0.78f);
	colors[ImGuiCol_TabDimmedSelectedOverline]  = ImVec4(0.50f, 0.50f, 0.50f, 0.00f);
	colors[ImGuiCol_DockingPreview]         = ImVec4(0.69f, 0.69f, 0.69f, 0.78f);
	colors[ImGuiCol_DockingEmptyBg]         = ImVec4(0.22f, 0.22f, 0.22f, 1.00f);
	colors[ImGuiCol_PlotLines]              = ImVec4(0.61f, 0.61f, 0.61f, 1.00f);
	colors[ImGuiCol_PlotLinesHovered]       = ImVec4(1.00f, 0.43f, 0.35f, 1.00f);
	colors[ImGuiCol_PlotHistogram]          = ImVec4(0.90f, 0.70f, 0.00f, 1.00f);
	colors[ImGuiCol_PlotHistogramHovered]   = ImVec4(1.00f, 0.60f, 0.00f, 1.00f);
	colors[ImGuiCol_TableHeaderBg]          = ImVec4(0.19f, 0.19f, 0.20f, 1.00f);
	colors[ImGuiCol_TableBorderStrong]      = ImVec4(0.29f, 0.29f, 0.29f, 1.00f);
	colors[ImGuiCol_TableBorderLight]       = ImVec4(0.29f, 0.29f, 0.29f, 0.50f);
	colors[ImGuiCol_TableRowBg]             = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
	colors[ImGuiCol_TableRowBgAlt]          = ImVec4(1.00f, 1.00f, 1.00f, 0.06f);
	colors[ImGuiCol_TextLink]               = ImVec4(0.29f, 0.50f, 1.00f, 1.00f);
	colors[ImGuiCol_TextSelectedBg]         = ImVec4(0.26f, 0.59f, 0.98f, 0.35f);
	colors[ImGuiCol_TreeLines]              = ImVec4(0.43f, 0.43f, 0.50f, 0.50f);
	colors[ImGuiCol_DragDropTarget]         = ImVec4(1.00f, 1.00f, 0.00f, 0.90f);
	colors[ImGuiCol_DragDropTargetBg]       = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
	colors[ImGuiCol_UnsavedMarker]          = ImVec4(0.69f, 0.69f, 0.69f, 1.00f);
	colors[ImGuiCol_NavCursor]              = ImVec4(0.98f, 0.98f, 0.98f, 1.00f);
	colors[ImGuiCol_NavWindowingHighlight]  = ImVec4(1.00f, 1.00f, 1.00f, 0.70f);
	colors[ImGuiCol_NavWindowingDimBg]      = ImVec4(0.80f, 0.80f, 0.80f, 0.20f);
	colors[ImGuiCol_ModalWindowDimBg]       = ImVec4(0.80f, 0.80f, 0.80f, 0.35f);
}
