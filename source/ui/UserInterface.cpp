#include "UserInterface.h"

UserInterface::UserInterface(SDL_Window* window, SDL_GLContext glContext) : state_(UIState::getInstance()) {
		
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImPlot::CreateContext();

	ImGui::StyleColorsDark();

	ImGui_ImplSDL3_InitForOpenGL(window, glContext);
	ImGui_ImplOpenGL3_Init();
	ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_DockingEnable;
	
	heatmapData_.reserve(512 * 32);
	heatmapData_.resize(512 * 32, 0.0);

	heatMapChange_.reserve(512 * 32);
	heatMapChange_.resize(512 * 32, 0.0);
}

UserInterface::~UserInterface() {
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplSDL3_Shutdown();
	ImPlot::DestroyContext();
	ImGui::DestroyContext();
}

void UserInterface::display(std::vector<double>& audioBuffer, std::vector<double>& spectrum, std::vector<double>& spectrumDiff, int bufferSize, bool hasNewSpectrumData) {
	// Start the Dear ImGui frame
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplSDL3_NewFrame();
	ImGui::NewFrame();

	ImGuiIO& guiIO = ImGui::GetIO();

	if(!guiIO.WantCaptureMouse) {
		state_.universalShaderSettings.mouseInputs = ImVec4(guiIO.MousePos.x, guiIO.MousePos.y, ImGui::IsMouseDown(0), ImGui::IsMouseDown(1));
	}

	mainMenuBarGUI();
	if(showNew_) {newModal();}
	if(showSlimeGUI_) {slimeGUI();}
	if(showVisualSettingsGUI_) {visualSettingsGUI();}
	if(showPresetGUI_) {presetGUI();}
	if(showAudioGUI_) {audioGUI(audioBuffer, spectrum, spectrumDiff, bufferSize, hasNewSpectrumData);}
	if(showDebugGUI_) {debugGUI(guiIO);}

	if(showImGUIDemo_) {ImGui::ShowDemoWindow();}
	if(showImPlotDemo_) {ImPlot::ShowDemoWindow();}
}

// Helper to display a little (?) mark which shows a tooltip when hovered.
// In your own code you may want to display an actual icon if you are using a merged icon fonts (see docs/FONTS.md)
static void HelpMarker(const char* desc)
{
    ImGui::TextDisabled("(?)");
    if (ImGui::BeginItemTooltip())
    {
        ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);
        ImGui::TextUnformatted(desc);
        ImGui::PopTextWrapPos();
        ImGui::EndTooltip();
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
		ImGui::InputInt("Number of Particles", &state_.newNumParticles, 8, 8, ImGuiInputTextFlags_CharsNoBlank);
		state_.newNumParticles = state_.newNumParticles - (state_.newNumParticles % 8);
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
            if (ImGui::MenuItem("Slime Config", "", showSlimeGUI_)) {showSlimeGUI_ = showSlimeGUI_ ? false : true;}
			if (ImGui::MenuItem("Visual Settings", "", showVisualSettingsGUI_)) {showVisualSettingsGUI_ = showVisualSettingsGUI_ ? false : true;}
			if (ImGui::MenuItem("Preset", "", showPresetGUI_)) {showPresetGUI_ = showPresetGUI_ ? false : true;}
            if (ImGui::MenuItem("Audio", "", showAudioGUI_)) {showAudioGUI_ = showAudioGUI_ ? false : true;}
			if (ImGui::MenuItem("Debug", "", showDebugGUI_)) {showDebugGUI_ = showDebugGUI_ ? false : true;}
			ImGui::Separator();
			if (ImGui::MenuItem("ImGui Demo", "", showImGUIDemo_)) {showImGUIDemo_ = showImGUIDemo_ ? false : true;}
			if (ImGui::MenuItem("ImPlot Demo", "", showImPlotDemo_)) {showImPlotDemo_ = showImPlotDemo_ ? false : true;}
            ImGui::EndMenu();
        }
        ImGui::EndMainMenuBar();
    }
}


void UserInterface::slimeGUI() {
	ImGui::Begin("Slime Config", &showSlimeGUI_);
	
	ImGui::SliderFloat("v", &state_.slimeSettings.v, 0.0f, 3.0f);
	ImGui::SliderInt("Rotation Angle", &state_.slimeSettings.rotationAngle, 0, 180);
	ImGui::SliderInt("Sensor Angle", &state_.slimeSettings.angle, 0, 180);
	
	ImGui::Checkbox("Lock Angles", &state_.lockAngles);
	ImGui::SliderInt("Sensor Distance", &state_.slimeSettings.sensorDistance, 1, 100);
	ImGui::SliderFloat("Deposition Strength", &state_.slimeSettings.depositionStrength, 0.0f, 10.0f);
	ImGui::SliderFloat("diffusionWeight", &state_.trailDiffusionSettings.diffusionWeight, 0.0f, 1.0f);
	ImGui::SliderFloat("decay", &state_.trailDiffusionSettings.decay, 0.0f, 1.0f);

	ImGui::Separator();
	
	ImGui::Checkbox("Lock Slime Color to Color 0", &state_.lockSlimeColor);
	
	ImGui::ColorEdit3("Slime Color 0", (float*)&state_.slimeSettings.slimeColor0);
	ImGui::ColorEdit3("Slime Color 1", (float*)&state_.slimeSettings.slimeColor1);
	ImGui::ColorEdit3("Slime Color 2", (float*)&state_.slimeSettings.slimeColor2);
	
	ImGui::Separator();
	
	ImGui::Checkbox("Use Particle Mask instead of Color", (bool*)&state_.slimeSettings.useMask);
	
	ImGui::Checkbox("Collision Detection", (bool*)&state_.universalShaderSettings.collisionDetection);

	ImGui::End();
	
	if (state_.lockAngles) {
		state_.slimeSettings.angle = state_.slimeSettings.rotationAngle;
	}
	
	if (state_.lockSlimeColor) {
		state_.slimeSettings.slimeColor1 = state_.slimeSettings.slimeColor0;
		state_.slimeSettings.slimeColor2 = state_.slimeSettings.slimeColor0;
	}
	
	if (state_.lockParticleColor) {
		state_.slimeSettings.particleColor1 = state_.slimeSettings.particleColor0;
		state_.slimeSettings.particleColor2 = state_.slimeSettings.particleColor0;
	}
}

void UserInterface::visualSettingsGUI() {

	ImGui::Begin("Visual Settings", &showVisualSettingsGUI_);

	ImGui::SliderFloat("Brightness Multiplier", &state_.fragmentShaderSettings.brightnessMultiplier, 0.0f, 10.0f);

	ImGui::Combo("Tone Mapping", (int*)&state_.fragmentShaderSettings.toneMappingMode, "Reinhard\0Exposure\0ACES\0");

	if(state_.fragmentShaderSettings.toneMappingMode == 1) {
		ImGui::SliderFloat("Exposure", &state_.fragmentShaderSettings.exposure, 0.0f, 5.0f);
	}
	ImGui::Separator();

	bool bloom = state_.fragmentShaderSettings.bloomEnabled != 0;
	if (ImGui::Checkbox("Bloom Effect", &bloom)) {
		state_.fragmentShaderSettings.bloomEnabled = bloom ? 1 : 0;
	}
	if (bloom) {
		ImGui::SliderFloat("Bloom Intensity", &state_.fragmentShaderSettings.bloomIntensity, 0.0f, 10.0f);
		ImGui::SliderFloat("Bloom Threshold", &state_.fragmentShaderSettings.bloomThreshold, 0.0f, 1.0f);
		ImGui::SameLine(); HelpMarker("Threshold how bright areas have to be to be considered for bloom.");
		ImGui::SliderFloat("Bloom Knee", &state_.fragmentShaderSettings.bloomKnee, 0.0f, 1.0f);
		ImGui::Combo("Bloom Blend Mode", (int*)&state_.fragmentShaderSettings.bloomBlendMode, "Additive\0Screen\0Soft Additive\0");
		ImGui::Separator();
	}
	
	// ImGui::Checkbox expects a bool*, but vignetteEffect is stored as an int in UIState;
	// use a temporary bool to interface with ImGui and write back the result to the int.
	bool vignette = state_.fragmentShaderSettings.vignetteEffect != 0;
	if (ImGui::Checkbox("Vignette Effect", &vignette)) {
		state_.fragmentShaderSettings.vignetteEffect = vignette ? 1 : 0;
	}
	ImGui::SameLine(); HelpMarker("When a square render is not desired, useful for example for projecting the image with a beamer onto a wall. Different shapes possible from a circle to very elongated almost straight ellipses.");

	if(state_.fragmentShaderSettings.vignetteEffect) {
		ImGui::SliderInt("Vignette Selector", &state_.fragmentShaderSettings.vignetteSelector, 0, 1);
		ImGui::SliderFloat("Vignette Sharpness", &state_.fragmentShaderSettings.vignetteSharpness, 1.0f, 10.0f);
		ImGui::SliderFloat("Vignette Inner Radius", &state_.fragmentShaderSettings.vignetteInnerRadius, 0.5f, 10.0f);
		ImGui::SliderFloat("Vignette X Dimension", &state_.fragmentShaderSettings.vignetteXDimension, 0.0f, 10.0f);
		ImGui::SliderFloat("Vignette Y Dimension", &state_.fragmentShaderSettings.vignetteYDimension, 0.0f, 10.0f);
		ImGui::Separator();
	}
	ImGui::Checkbox("Fullscreen", &state_.fullscreen);

	ImGui::End();
}

void UserInterface::presetGUI() {
	ImGui::Begin("Preset", &showPresetGUI_);
	//--------------------------------
	//Preset System
	//--------------------------------
	static char presetNameChar[128] = "";
	ImGui::InputTextWithHint("Preset Name", "Preset Name", presetNameChar, IM_ARRAYSIZE(presetNameChar));

	static bool presetAlreadyExists = false;

	if (ImGui::Button("Save Preset") && presetNameChar[0] != '\0') {

		for(unsigned int i = 0; i < presetNames_.size(); i++) {
			if(presetNames_[i] == std::string(presetNameChar)) {
				presetAlreadyExists = true;
			}
		}

		if(!presetAlreadyExists) {
			addPresetName(std::string(presetNameChar));
			state_.saveToPreset = true;
		}

		presetNameChar[0] = '\0';
	}

	if(presetAlreadyExists && ImGui::IsItemHovered()){
		ImGui::SameLine();
		ImGui::Text("Preset with this name already exists!");
	} else {
		presetAlreadyExists = false;
	}

	if (ImGui::BeginListBox("Presets")) {

		for (unsigned int n = 0; n < presetNames_.size(); n++)
		{
			const bool is_selected = (selectedPresetName_ == n);
			if (ImGui::Selectable(presetNames_[n].c_str(), is_selected)) {
				selectedPresetName_ = n;
				state_.loadFromPreset = true;
				std::cout << "Selected Preset: " << presetNames_[n] << std::endl;
			}
			// Set the initial focus when opening the combo (scrolling + keyboard navigation focus)
			if (is_selected) {
				ImGui::SetItemDefaultFocus();
			}
		}

		ImGui::EndListBox();
	}

	//--------------------------------
	//Color Preset System
	//--------------------------------
	static char colorPresetNameChar[128] = "";
	ImGui::InputTextWithHint("Color Preset Name", "Color Preset Name", colorPresetNameChar, IM_ARRAYSIZE(colorPresetNameChar));

	static bool colorPresetAlreadyExists = false;

	if (ImGui::Button("Save Color Preset") && colorPresetNameChar[0] != '\0') {

		for(unsigned int i = 0; i < colorPresetNames_.size(); i++) {
			if(colorPresetNames_[i] == std::string(colorPresetNameChar)) {
				colorPresetAlreadyExists = true;
			}
		}

		if(!colorPresetAlreadyExists) {
			addColorPresetName(std::string(colorPresetNameChar));
			state_.saveToColorPreset = true;
		}

		colorPresetNameChar[0] = '\0';
	}

	if(colorPresetAlreadyExists && ImGui::IsItemHovered()){
		ImGui::SameLine();
		ImGui::Text("Color Preset with this name already exists!");
	} else {
		colorPresetAlreadyExists = false;
	}

	if (ImGui::BeginListBox("Color Presets")) {

		for (unsigned int n = 0; n < colorPresetNames_.size(); n++)
		{
			const bool is_selected = (selectedColorPresetName_ == n);
			if (ImGui::Selectable(colorPresetNames_[n].c_str(), is_selected)) {
				selectedColorPresetName_ = n;
				state_.loadFromColorPreset = true;
				std::cout << "Selected Color Preset: " << colorPresetNames_[n] << std::endl;
			}
			// Set the initial focus when opening the combo (scrolling + keyboard navigation focus)
			if (is_selected) {
				ImGui::SetItemDefaultFocus();
			}
		}

		ImGui::EndListBox();
	}

	ImGui::Checkbox("Auto Preset Switching", &state_.autoPresetSwitching);
	ImGui::SliderInt("Preset Intervall [s]", &state_.presetIntervall, 2, 60);
	ImGui::SliderInt("Switch at Beat Volume", &state_.beatVolumeSwitch, 0, 50);
	// if (ImGui::Button("Delete Preset")) {
	// 	state_.deletePreset = true;
	// }

	ImGui::End();
}

void UserInterface::audioGUI(std::vector<double>& audioBuffer, std::vector<double>& spectrum, std::vector<double>& spectrumDiff, int bufferSize, bool hasNewSpectrumData) {
	ImGui::Begin("Audio", &showAudioGUI_);

	ImGui::Checkbox("Enable Audio Processing", (bool*)&state_.slimeSettings.reactToAudio);
	ImGui::SameLine(); HelpMarker("When enabled, the slime movement will react to audio input. Make sure to select an audio input device in the Audio menu.");
	
    if (ImGui::CollapsingHeader("Audio Graph")) {
		if (ImPlot::BeginPlot("Audio")) {
			ImPlot::SetupAxes("t","y", ImPlotAxisFlags_None, ImPlotAxisFlags_AutoFit);
			ImPlot::SetupAxisFormat(ImAxis_X1, TimeFormatter, (void*)"s");
			ImPlot::SetupFinish();
			ImPlot::PlotLineG("Audio##2", MyDataGetter, audioBuffer.data(), bufferSize);
			//ImPlot::PlotLine("Audio", audioBuffer.data(), bufferSize);
			ImPlot::SetNextMarkerStyle(ImPlotMarker_Circle);
			ImPlot::EndPlot();
		}
	}
	if (ImGui::CollapsingHeader("Audio Spectrum")) {

		if (ImPlot::BeginPlot("Audio Spectrum##2")) {
			ImPlot::SetupAxes("Frequency (Hz)", "Magnitude", ImPlotAxisFlags_None, ImPlotAxisFlags_AutoFit);
			//static ImPlotDragToolFlags flags = ImPlotDragToolFlags_None;
			//ImPlot::SetupAxisLimits(ImAxis_Y1, 0, 700, ImGuiCond_Always);
			ImPlot::SetupAxisFormat(ImAxis_X1, FrequencyFormatter, (void*)"Hz");
			//ImPlot::SetupAxisScale(ImAxis_Y1, ImPlotScale_Log10);  // Log scale for magnitude
			//double beatBeginnFreq = 11.0f;//state_.beatBeginn * (1 / (bufferSize / (double)audioRate));
			ImPlot::SetupFinish();
			ImPlot::PlotLine("Spectrum", spectrum.data(), 2048);
			//ImPlot::PlotInfLines("BeatRange", beatRange, 2);
			//ImPlot::PlotInfLines("FringeRange", fringeRange, 2);
			//ImPlot::PlotInfLines("BeatLimits", beatLimits, 2, ImPlotInfLinesFlags_Horizontal);
			//ImPlot::PlotBars("Spectrum", spectrum.data(), 100, 1.0, 0.5);

			ImPlot::EndPlot();
		}
	}

	// Heatmap
	if(hasNewSpectrumData)  {
		for (size_t i = 0; i < 512 * 32; i = i + 32) {
			heatmapData_[i + (static_cast<size_t>(heatMapIndex_))] = spectrum[i / 32];
		}

		for (size_t i = 0; i < 512 * 32; i = i + 32) {
			heatMapChange_[i + (static_cast<size_t>(heatMapIndex_))] = spectrumDiff[i / 32] > 0 ? spectrumDiff[i / 32] : 0.0;
		}

		heatMapIndex_ = (heatMapIndex_ + 1) % 32;
	}

    static float scale_min       = 0.0f;
    static float scale_max       = 100;
    //static const char* xlabels[] = {"C1","C2","C3","C4","C5","C6","C7"};
    //static const char* ylabels[] = {"R1","R2","R3","R4","R5","R6","R7"};

    static ImPlotColormap map = ImPlotColormap_Hot;

    ImGui::SameLine();
    ImGui::LabelText("##Colormap Index", "%s", "Change Colormap");
    ImGui::SetNextItemWidth(225);
    ImGui::DragFloatRange2("Min / Max",&scale_min, &scale_max, 0.01f, -20, 20);

    static ImPlotAxisFlags axes_flags = ImPlotAxisFlags_Lock | ImPlotAxisFlags_NoGridLines | ImPlotAxisFlags_NoTickMarks;

	ImPlot::PushColormap(map);

	if (ImPlot::BeginPlot("##Heatmap1",ImVec2(512,768),ImPlotFlags_NoLegend|ImPlotFlags_NoMouseText)) {
		ImPlot::SetupAxes("Time", "Frequency (Hz)", axes_flags, axes_flags);
        //ImPlot::SetupAxisTicks(ImAxis_X1,0 + 1.0/14.0, 1 - 1.0/14.0, 7, xlabels);
		ImPlot::SetupFinish();
        ImPlot::PlotHeatmap("heat", heatmapData_.data(), 512, 32, scale_min, scale_max, nullptr, ImPlotPoint(0,1), ImPlotPoint(1,0), 0);
        ImPlot::EndPlot();
    }

	ImGui::SameLine();

	if (ImPlot::BeginPlot("##Heatmap2",ImVec2(512,768),ImPlotFlags_NoLegend|ImPlotFlags_NoMouseText)) {
		ImPlot::SetupAxes("Time", "Frequency (Hz)", axes_flags, axes_flags);
        //ImPlot::SetupAxisTicks(ImAxis_X1,0 + 1.0/14.0, 1 - 1.0/14.0, 7, xlabels);
		ImPlot::SetupFinish();
        ImPlot::PlotHeatmap("heat", heatMapChange_.data(), 512, 32, scale_min, scale_max, nullptr, ImPlotPoint(0,1), ImPlotPoint(1,0), 0);
        ImPlot::EndPlot();
    }
	ImPlot::PopColormap();	//to avoid interfering with the colour of other plots when the draw order changes (Collapsing Headers, ...)

	// ImGui::SliderInt("Scan Time [ms]", &audioTimer, 1, 1000);
	// if (ImGui::IsItemDeactivatedAfterEdit()) {
	// 	std::cout << "Hi" << std::endl;
	// 	SDL_RemoveTimer(timerID);
	// 	timerID = SDL_AddTimer(audioTimer, getAudioCallback, &data);
	// }

	// float minFreqency = 1 / (BUFFER_SIZE / (double)AudioRate);

	ImGui::SeparatorText("Beat Detection");

	ImGui::Text("VelocityBassReaction: %.2f", state_.slimeSettings.velocityBassReaction);

	HelpMarker("Frequency Range: 20-60 Hz");
	ImGui::SameLine();
	ImGui::Text("Sub Bass Detected: %d", state_.subBassDetected);
	ImGui::SameLine();
	ImGui::Text("Sub Bass Value: %.2f", state_.subBassValue);

	HelpMarker("Frequency Range: 60-250 Hz");
	ImGui::SameLine();
	ImGui::Text("Bass Detected: %d", state_.bassDetected);
	ImGui::SameLine();
	ImGui::Text("Bass Value: %.2f", state_.bassValue);

	HelpMarker("Frequency Range: 250-500 Hz");
	ImGui::SameLine();
	ImGui::Text("Low Mid Range Detected: %d", state_.lowMidRangeDetected);
	ImGui::SameLine();
	ImGui::Text("Low Mid Range Value: %.2f", state_.lowMidRangeValue);

	HelpMarker("Frequency Range: 500-2000 Hz");
	ImGui::SameLine();
	ImGui::Text("Mid Range Detected: %d", state_.midRangeDetected);
	ImGui::SameLine();
	ImGui::Text("Mid Range Value: %.2f", state_.midRangeValue);

	HelpMarker("Frequency Range: 2000-4000 Hz");
	ImGui::SameLine();
	ImGui::Text("Upper Mid Range Detected: %d", state_.upperMidRangeDetected);
	ImGui::SameLine();
	ImGui::Text("Upper Mid Range Value: %.2f", state_.upperMidRangeValue);

	HelpMarker("Frequency Range: 4000-6000 Hz");
	ImGui::SameLine();
	ImGui::Text("Presence Detected: %d", state_.presenceDetected);
	ImGui::SameLine();
	ImGui::Text("Presence Value: %.2f", state_.presenceValue);

	HelpMarker("Frequency Range: 6000-20000 Hz");
	ImGui::SameLine();
	ImGui::Text("Brilliance Detected: %d", state_.brillianceDetected);
	ImGui::SameLine();
	ImGui::Text("Brilliance Value: %.2f", state_.brillianceValue);

	ImGui::SliderFloat("Beat Divide", &state_.beatDivide, 0, 100.0f);
	ImGui::Checkbox("Beat Normalization", &state_.normalizeBeat);
	// ImGui::SeparatorText("Fringe Detection (non functional)");
	// ImGui::SliderInt("Fringe Beginn", &state_.fringeBeginn, 0, state_.fringeEnd);
	// ImGui::SliderInt("Fringe End", &state_.fringeEnd, state_.fringeBeginn, bufferSize / 2);
	// ImGui::SliderInt("Fringe Divide", &state_.fringeDivide, 1, 1000);
	
	if (ImGui::BeginListBox("Audio Hardware")) {

		for (unsigned int n = 0; n < availableHardwareDevices_.size(); n++)
		{
			const bool is_selected = (selectedHardwareDevice_ == n);
			if (ImGui::Selectable(availableHardwareDevices_[n].c_str(), is_selected)) {
				selectedHardwareDevice_ = n;
				state_.selectAudioHardware = true;
			}
			// Set the initial focus when opening the combo (scrolling + keyboard navigation focus)
			if (is_selected) {
				ImGui::SetItemDefaultFocus();
			}
		}

		ImGui::EndListBox();
	}

	ImGui::End();
}

void UserInterface::debugGUI(ImGuiIO& guiIO) {
	ImGui::Begin("Debug", &showDebugGUI_);

	ImGui::Checkbox("Render Particles", (bool*)&state_.universalShaderSettings.renderParticles);
	ImGui::Checkbox("Lock Particle Color to Color 0", &state_.lockParticleColor);
	ImGui::ColorEdit3("Particle Color 0", (float*)&state_.slimeSettings.particleColor0);
	ImGui::ColorEdit3("Particle Color 1", (float*)&state_.slimeSettings.particleColor1);
	ImGui::ColorEdit3("Particle Color 2", (float*)&state_.slimeSettings.particleColor2);

	ImGui::Separator();

	ImGui::Checkbox("Render Collisions", (bool*)&state_.universalShaderSettings.renderCollisions);
	if(state_.universalShaderSettings.renderCollisions) {
		ImGui::SliderFloat("Collision Fraction", &state_.slimeSettings.collisionFraction, 0.0f, 1.0f);
		ImGui::ColorEdit3("Collision Color", (float*)&state_.slimeSettings.collisionColor);
	}

	ImGui::Separator();

	// ImGui::Checkbox expects a bool*, but renderColorTraces is stored as an int in UIState;
	// use a temporary bool to interface with ImGui and write back the result to the int.
	bool renderColorTraces = state_.fragmentShaderSettings.renderColorTraces != 0;
	if (ImGui::Checkbox("Render Color Traces", &renderColorTraces)) {
		state_.fragmentShaderSettings.renderColorTraces = renderColorTraces ? 1 : 0;
	}

	ImGui::ColorEdit4("Clear Color", (float*)&state_.clearColor);

    if (ImGui::BeginListBox("Texture Mask")) {
        const char* textureMaskNames[] = {
            "Trail Texture",
            "Trail Non-Diffused",
            "New Particles",
            "Old Particles",
            "Collisions",
            "Bloom",
			"UpSample 1",
			"DownSample 2",
			"UpSample 2",
			"DownSample 3",
			"UpSample 3",
			"DownSample 4",
			"UpSample 4",
			"DownSample 5",
			"UpSample 5",
			"Threshold"
        };

        for (int n = 0; n < IM_ARRAYSIZE(textureMaskNames); n++) {
            const bool is_selected = (state_.fragmentShaderSettings.debugTextureMaskSelector == n);
            if (ImGui::Selectable(textureMaskNames[n], is_selected)) {
                state_.fragmentShaderSettings.debugTextureMaskSelector = n;
                state_.selectedTextureMask = static_cast<TextureMask>(n);
            }
            
            // Set the initial focus when opening the combo (scrolling + keyboard navigation focus)
            if (is_selected) {
                ImGui::SetItemDefaultFocus();
            }
        }

        ImGui::EndListBox();
    }

	ImGui::Text("WindowWidth: %d TextureWidth: %d NewTextureWidth: %d Height: %d TextureHeight: %d NewTextureHeight: %d", state_.universalShaderSettings.windowWidth, state_.universalShaderSettings.textureWidth, state_.newTextureWidth, state_.universalShaderSettings.windowHeight, state_.universalShaderSettings.textureHeight, state_.newTextureHeight);
	ImGui::Text("ShowSlime: %d ShowAudio: %d ShowPreset: %d ShowDebug: %d ", showSlimeGUI_, showAudioGUI_, showPresetGUI_, showDebugGUI_);
	ImGui::Text("Fullscreen: %d", state_.fullscreen);

	if (ImGui::IsMousePosValid()) {
		ImGui::Text("Mouse pos: (%g, %g)", guiIO.MousePos.x, guiIO.MousePos.y);
	}
	else{
		ImGui::Text("Mouse pos: <INVALID>");
	}
	ImGui::Text("Mouse delta: (%g, %g)", guiIO.MouseDelta.x, guiIO.MouseDelta.y);
	ImGui::Text("Mouse down:");
	for (int i = 0; i < IM_ARRAYSIZE(guiIO.MouseDown); i++) {
		if (ImGui::IsMouseDown(i)) { 
			ImGui::SameLine(); ImGui::Text("b%d (%.02f secs)", i, guiIO.MouseDownDuration[i]); 
		}
	}
	ImGui::Text("Mouse wheel: %.1f", guiIO.MouseWheel);
	
	ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);

	ImGui::End();

	if (state_.lockParticleColor) {
		state_.slimeSettings.particleColor1 = state_.slimeSettings.particleColor0;
		state_.slimeSettings.particleColor2 = state_.slimeSettings.particleColor0;
	}
}

ImPlotPoint MyDataGetter(int idx, void* data) {
	double* my_data = (double*)data;
	ImPlotPoint p;
	p.x = idx;
	p.y = (double)my_data[idx];
	return p;
}

int TimeFormatter(double value, char* buff, int size, void* data) {
    const char* unit = (const char*)data;
    static double v[]      = {1000000000,1000000,1000,1,0.001,0.000001,0.000000001};
    static const char* p[] = {"G","M","k","","m","u","n"};
	value = value / 48000;
	if (fabs(value) < std::numeric_limits<double>::epsilon()) {
		return snprintf(buff,static_cast<size_t>(size),"0 %s", unit);
	}
    for (int i = 0; i < 7; ++i) {
        if (fabs(value) >= v[i]) {
            return snprintf(buff,static_cast<size_t>(size),"%g %s%s",value/v[i],p[i],unit);
        }
    }
    return snprintf(buff,static_cast<size_t>(size),"%g %s%s",value/v[6],p[6],unit);
}

int FrequencyFormatter(double value, char* buff, int size, void* data) {
    const char* unit = (const char*)data;
    static double v[]      = {1000000000,1000000,1000,1,0.001,0.000001,0.000000001};
    static const char* p[] = {"G","M","k","","m","u","n"};
	value = value * (1 / (4096 / (double)48000));
    if (fabs(value) < std::numeric_limits<double>::epsilon()) {
        return snprintf(buff,static_cast<size_t>(size),"0 %s", unit);
    }
    for (int i = 0; i < 7; ++i) {
        if (fabs(value) >= v[i]) {
            return snprintf(buff,static_cast<size_t>(size),"%g %s%s",value/v[i],p[i],unit);
        }
    }
    return snprintf(buff,static_cast<size_t>(size),"%g %s%s",value/v[6],p[6],unit);
}
