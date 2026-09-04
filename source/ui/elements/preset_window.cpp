#include "preset_window.h"

#include "../../utility/event.h"
#include "../../simulation/trail_map_controller.h"
#include "misc/cpp/imgui_stdlib.cpp"	//for string parameters in text input fields

void PresetWindow::render(ApplicationState* appState) {
    if(!visible) { return; }

	ImGui::Begin("Preset", &visible);

	if (ImGui::BeginTabBar("Preset Types")) {
		if (ImGui::BeginTabItem("Behaviour")) {
			behaviourPresetGUI(appState);
			ImGui::EndTabItem();
		}
		if (ImGui::BeginTabItem("Color")) {
			colorPresetGUI(appState);
			ImGui::EndTabItem();
		}
		if (ImGui::BeginTabItem("Image")) {
			imagePresetGUI(appState);
			ImGui::EndTabItem();
		}
		if (ImGui::BeginTabItem("Text")) {
			textPresetGUI(appState);
			ImGui::EndTabItem();
		}
		ImGui::EndTabBar();
	}
	ImGui::SeparatorText("Shared Options");
	ImGui::Checkbox("Auto Preset Switching", &appState->autoPresetSwitching);
	ImGui::SliderInt("Switch at Beat Volume", &appState->beatVolumeSwitch, 0, 50);

	ImGui::End();
}

void PresetWindow::behaviourPresetGUI(ApplicationState* appState) {
	//--------------------------------
	//Preset System
	//--------------------------------
	static char presetNameChar[128] = "";
	ImGui::InputTextWithHint("Preset Name", "Preset Name", presetNameChar, IM_ARRAYSIZE(presetNameChar), ImGuiInputTextFlags_CharsNoBlank);

	static bool presetAlreadyExists = false;

	if (ImGui::Button("Save Preset") && presetNameChar[0] != '\0') {

		for(unsigned int i = 0; i < presetNames_.size(); i++) {
			if(presetNames_[i] == std::string(presetNameChar)) {
				presetAlreadyExists = true;
			}
		}

		if(!presetAlreadyExists) {
			addPresetName(std::string(presetNameChar));
			notify(UserEvent{EventType::SAVE_PRESET, 0});
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
				notify(UserEvent{EventType::LOAD_PRESET, 0});
				std::cout << "Selected Preset: " << presetNames_[n] << std::endl;
			}
			// Set the initial focus when opening the combo (scrolling + keyboard navigation focus)
			if (is_selected) {
				ImGui::SetItemDefaultFocus();
			}
		}

		ImGui::EndListBox();
	}

	ImGui::SliderInt("Preset Time Intervall [s]", &appState->presetIntervall, 2, 60);
}

void PresetWindow::colorPresetGUI(ApplicationState* appState) {
	//--------------------------------
	//Color Preset System
	//--------------------------------
	static char colorPresetNameChar[128] = "";
	ImGui::InputTextWithHint("Color Preset Name", "Color Preset Name", colorPresetNameChar, IM_ARRAYSIZE(colorPresetNameChar), ImGuiInputTextFlags_CharsNoBlank);

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
			notify(UserEvent{EventType::SAVE_COLOR_PRESET, 0});
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
				notify(UserEvent{EventType::LOAD_COLOR_PRESET, 0});
				std::cout << "Selected Color Preset: " << colorPresetNames_[n] << std::endl;
			}
			// Set the initial focus when opening the combo (scrolling + keyboard navigation focus)
			if (is_selected) {
				ImGui::SetItemDefaultFocus();
			}
		}

		ImGui::EndListBox();
	}

	ImGui::SliderInt("Color Preset Time Intervall [s]", &appState->colorPresetIntervall, 2, 60);
}

void PresetWindow::imagePresetGUI(ApplicationState* appState) {
	//--------------------------------
	//Picture Selection
	//--------------------------------
	if (ImGui::BeginListBox("Pictures")) {
		std::vector<TrailMask>* trailMasks = appState->trailMasks;
		size_t usedTrailMaskIndex = appState->usedTrailMaskIndex;
		if(trailMasks != nullptr) {
			for (unsigned int i = 0; i < trailMasks->size(); ++i) {
				const bool is_selected = (usedTrailMaskIndex == i);
				bool isImageMask = !((*trailMasks)[i].isText);
				if (isImageMask && ImGui::Selectable((*trailMasks)[i].imageName.c_str(), is_selected)) {
					appState->usedTrailMaskIndex = i;
					notify(UserEvent{EventType::LOAD_NEW_PICTURE, 0});
					std::cout << "Selected Picture: " << (*trailMasks)[i].imageName << std::endl;
				}
				// Set the initial focus when opening the combo (scrolling + keyboard navigation focus)
				if (is_selected) {
					ImGui::SetItemDefaultFocus();
				}
			}
		}

		ImGui::EndListBox();
	}

	ImGui::SliderFloat("Trail Mask Influence", &appState->universalShaderSettings.trailMaskInfluence, 0.0f, 5.0f);
	ImGui::SliderFloat("Trail Mask Scale", &appState->universalShaderSettings.trailMaskScale, 0.1f, 10.0f);
	ImGui::SliderInt("Trail Mask Time Intervall [s]", &appState->trailMaskIntervall, 2, 60);
}

void PresetWindow::textPresetGUI(ApplicationState* appState) {
	//--------------------------------
	//Text Selection
	//--------------------------------

	if (ImGui::BeginListBox("Text Presets")) {
		std::vector<TrailMask>* trailMasks = appState->trailMasks;
		size_t usedTrailMaskIndex = appState->usedTrailMaskIndex;
		if(trailMasks != nullptr) {
			for (unsigned int i = 0; i < trailMasks->size(); ++i) {
				const bool is_selected = (usedTrailMaskIndex == i);
				bool isTextMask = (*trailMasks)[i].isText;
				if (isTextMask && ImGui::Selectable((*trailMasks)[i].imageName.c_str(), is_selected)) {
					appState->usedTrailMaskIndex = i;
					notify(UserEvent{EventType::LOAD_NEW_PICTURE, 0});
					std::cout << "Selected Picture: " << (*trailMasks)[i].imageName << std::endl;
				}
				//uses last item as Popup ID
                if (ImGui::BeginPopupContextItem()) {

					ImGui::Text("Edit text:");
					ImGui::InputText("##edittext", &(appState->textPreset));
					if (ImGui::Button("OK")) {
						notify(UserEvent{EventType::EDIT_TEXT_TEXTURE, 0});
						ImGui::CloseCurrentPopup();
					}
					ImGui::SameLine();
					if (ImGui::Button("Cancel")) {
						ImGui::CloseCurrentPopup();
					}
					ImGui::SameLine();
					if (ImGui::Button("Delete")) {
						notify(UserEvent{EventType::DELETE_TEXT_TEXTURE, 0});
						ImGui::CloseCurrentPopup();
					}
					ImGui::EndPopup();
                }

				// Set the initial focus when opening the combo (scrolling + keyboard navigation focus)
				if (is_selected) {
					ImGui::SetItemDefaultFocus();
				}

				ImGui::SetItemTooltip("Right-click to Edit or Delete");
			}
		}

		ImGui::EndListBox();
	}
	if(ImGui::Button("New Text")) { ImGui::OpenPopup("New Text"); }

	if(ImGui::BeginPopupModal("New Text", NULL, ImGuiWindowFlags_AlwaysAutoResize)) {
		ImGui::Text("Enter new text:");
		static char newTextBuffer[32] = "";
		ImGui::InputText("##newtext", newTextBuffer, IM_COUNTOF(newTextBuffer));
		if (ImGui::Button("OK")) {
			appState->textPreset = std::string(newTextBuffer);
			notify(UserEvent{EventType::CREATE_NEW_TEXT_TEXTURE, 0});
			ImGui::CloseCurrentPopup();
		}
		ImGui::SameLine();
		if (ImGui::Button("Cancel")) {
			ImGui::CloseCurrentPopup();
		}
		ImGui::EndPopup();
	}

	ImGui::Button("Edit Text");
}