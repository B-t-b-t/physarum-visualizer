#include "preset_window.h"

void PresetWindow::render(ApplicationState* appState) {
    if(!visible) { return; }

	ImGui::Begin("Preset", &visible);
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
			notify(Event::SAVE_PRESET);
			//appState->saveToPreset = true;
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
				//appState->loadFromPreset = true;
				notify(Event::LOAD_PRESET);
				std::cout << "Selected Preset: " << presetNames_[n] << std::endl;
			}
			// Set the initial focus when opening the combo (scrolling + keyboard navigation focus)
			if (is_selected) {
				ImGui::SetItemDefaultFocus();
			}
		}

		ImGui::EndListBox();
	}

	ImGui::Separator();

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
			//appState->saveToColorPreset = true;
			notify(Event::SAVE_COLOR_PRESET);
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
				//appState->loadFromColorPreset = true;
				notify(Event::LOAD_COLOR_PRESET);
				std::cout << "Selected Color Preset: " << colorPresetNames_[n] << std::endl;
			}
			// Set the initial focus when opening the combo (scrolling + keyboard navigation focus)
			if (is_selected) {
				ImGui::SetItemDefaultFocus();
			}
		}

		ImGui::EndListBox();
	}

	ImGui::Separator();

	//--------------------------------
	//Picture Selection
	//--------------------------------
	if (ImGui::BeginListBox("Pictures")) {

		for (unsigned int n = 0; n < pictureNames_.size(); n++)
		{
			const bool is_selected = (selectedPictureName_ == n);
			if (ImGui::Selectable(pictureNames_[n].c_str(), is_selected)) {
				selectedPictureName_ = n;
				//appState->loadNewPicture = true;
				notify(Event::LOAD_NEW_PICTURE);
				std::cout << "Selected Picture: " << pictureNames_[n] << std::endl;
			}
			// Set the initial focus when opening the combo (scrolling + keyboard navigation focus)
			if (is_selected) {
				ImGui::SetItemDefaultFocus();
			}
		}

		ImGui::EndListBox();
	}

	ImGui::SliderFloat("Trail Mask Influence", &appState->universalShaderSettings.trailMaskInfluence, 0.0f, 5.0f);
	ImGui::SliderFloat("Trail Mask Scale", &appState->universalShaderSettings.trailMaskScale, 0.1f, 10.0f);
	ImGui::SliderInt("Trail Mask Time Intervall [s]", &appState->trailMaskIntervall, 2, 60);

	ImGui::Separator();

	ImGui::Checkbox("Auto Preset Switching", &appState->autoPresetSwitching);
	ImGui::SliderInt("Preset Time Intervall [s]", &appState->presetIntervall, 2, 60);
	ImGui::SliderInt("Color Preset Time Intervall [s]", &appState->colorPresetIntervall, 2, 60);
	ImGui::SliderInt("Switch at Beat Volume", &appState->beatVolumeSwitch, 0, 50);

	ImGui::End();
}