#include "preset_window.h"

#include "../../utility/event.h"
#include "../../simulation/trail_map_controller.h"
#include "misc/cpp/imgui_stdlib.cpp"	//for string parameters in text input fields
#include "link_element.h"

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
			notify(UserEvent{EventType::SAVE_PRESET, 0, 0});
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
				notify(UserEvent{EventType::LOAD_PRESET, 0, 0});
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
			notify(UserEvent{EventType::SAVE_COLOR_PRESET, 0, 0});
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
				notify(UserEvent{EventType::LOAD_COLOR_PRESET, 0, 0});
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
    static bool editTimeSlot = false;
    static int editDayBegin = 1;
    static int editHourBegin = 0;
    static int editMinuteBegin = 0;
    static int editDayEnd = 1;
    static int editHourEnd = 0;
    static int editMinuteEnd = 0;

    if(ImGui::BeginListBox("Pictures")) {
        std::vector<TrailMask>* trailMasks = appState->trailMasks;
        const size_t usedTrailMaskIndex = appState->usedTrailMaskIndex;

        if(trailMasks != nullptr) {
            for(unsigned int i = 0; i < trailMasks->size(); ++i) {
                TrailMask& trailMask = (*trailMasks)[i];

                if(trailMask.isText) {
                    continue;
                }

                ImGui::PushID(static_cast<int>(i));

				//display clock symbol (0xF017) if entry has a time slot
				const bool isSelected = (usedTrailMaskIndex == i);
				const std::string displayName = trailMask.imageName +
					(trailMask.hasTimeSlot ? "  " : "");

				if(ImGui::Selectable(displayName.c_str(), isSelected)) {
					appState->usedTrailMaskIndex = i;
					notify(UserEvent{EventType::LOAD_NEW_PICTURE, 0, 0});
					std::cout << "Selected Picture: " << trailMask.imageName << std::endl;
				}

                if(ImGui::IsItemClicked(ImGuiMouseButton_Right)) {
                    editTimeSlot = trailMask.hasTimeSlot;
                    editDayBegin = 1;
                    editHourBegin = 0;
                    editMinuteBegin = 0;
                    editDayEnd = 1;
                    editHourEnd = 0;
                    editMinuteEnd = 0;

                    SDL_DateTime beginDateTime{};
                    SDL_DateTime endDateTime{};

                    if(trailMask.hasTimeSlot &&
                       SDL_TimeToDateTime(trailMask.beginTimeSlot, &beginDateTime, true) &&
                       SDL_TimeToDateTime(trailMask.endTimeSlot, &endDateTime, true)) {
                        editDayBegin = static_cast<int>(beginDateTime.day);
                        editHourBegin = static_cast<int>(beginDateTime.hour);
                        editMinuteBegin = static_cast<int>(beginDateTime.minute);
                        editDayEnd = static_cast<int>(endDateTime.day);
                        editHourEnd = static_cast<int>(endDateTime.hour);
                        editMinuteEnd = static_cast<int>(endDateTime.minute);
                    }
                }

                if(ImGui::BeginPopupContextItem("ImagePresetContext")) {
                    ImGui::Text("Edit time slot for: %s", trailMask.imageName.c_str());
                    ImGui::Separator();

                    ImGui::Checkbox("Time Slot", &editTimeSlot);

                    if(editTimeSlot) {
                        ImGui::Text("Begin:");
                        ImGui::DragInt("Day Begin", &editDayBegin, 1, 1, 31);
                        ImGui::DragInt("Hour Begin", &editHourBegin, 1, 0, 23);
                        ImGui::DragInt("Minute Begin", &editMinuteBegin, 1, 0, 59);

                        ImGui::Text("End:");
                        ImGui::DragInt("Day End", &editDayEnd, 1, 1, 31);
                        ImGui::DragInt("Hour End", &editHourEnd, 1, 0, 23);
                        ImGui::DragInt("Minute End", &editMinuteEnd, 1, 0, 59);
                    }

                    ImGui::Separator();

                    if(ImGui::Button("OK")) {
                        TrailMaskData trailMaskData{
							.newName = trailMask.imageName,
							.isText = false,
                            .hasTimeSlot = editTimeSlot,
                            .dayBegin = editDayBegin,
                            .hourBegin = editHourBegin,
                            .minuteBegin = editMinuteBegin,
                            .dayEnd = editDayEnd,
                            .hourEnd = editHourEnd,
                            .minuteEnd = editMinuteEnd
                        };

                        notify(UserEvent{
                            EventType::EDIT_TRAIL_MASK_TIME_SLOT,
                            static_cast<int>(i),
                            trailMaskData
                        });

                        ImGui::CloseCurrentPopup();
                    }

                    ImGui::SameLine();

                    if(ImGui::Button("Cancel")) {
                        ImGui::CloseCurrentPopup();
                    }

                    ImGui::EndPopup();
                }

                if(isSelected) {
                    ImGui::SetItemDefaultFocus();
                }

                ImGui::SetItemTooltip("Right-click to edit the time slot");
                ImGui::PopID();
            }
        }

        ImGui::EndListBox();
    }

    ImGui::SliderFloat(
        "Trail Mask Influence",
        &appState->universalShaderSettings.trailMaskInfluence,
        0.0f,
        5.0f
    );

    if(ImGui::SliderFloat(
        "Trail Mask Scale X",
        &appState->universalShaderSettings.trailMaskScaleX,
        0.1f,
        10.0f
    ) && linkTrailMaskScales_) {
        appState->universalShaderSettings.trailMaskScaleY =
            appState->universalShaderSettings.trailMaskScaleX;
    }
    
    if(ImGui::SliderFloat(
        "Trail Mask Scale Y",
        &appState->universalShaderSettings.trailMaskScaleY,
        0.1f,
        10.0f
    ) && linkTrailMaskScales_) {
        appState->universalShaderSettings.trailMaskScaleX =
        appState->universalShaderSettings.trailMaskScaleY;
    }

    if(ImGui::link("Link##first_second", &linkTrailMaskScales_, 2, 2.0f)) {
        appState->universalShaderSettings.trailMaskScaleY =
        appState->universalShaderSettings.trailMaskScaleX;
    }

    ImGui::SliderFloat(
        "Trail Mask Position X",
        &appState->universalShaderSettings.trailMaskPosition.x,
        -1.0f,
        1.0f
    );
    ImGui::SliderFloat(
        "Trail Mask Position Y",
        &appState->universalShaderSettings.trailMaskPosition.y,
        -1.0f,
        1.0f
    );
    ImGui::SliderInt(
        "Trail Mask Time Intervall [s]",
        &appState->trailMaskIntervall,
        2,
        60
    );
}

void PresetWindow::textPresetGUI(ApplicationState* appState) {
    //--------------------------------
    //Text Selection
    //--------------------------------
    static bool editTimeSlot = false;
    static int editDayBegin = 1;
    static int editHourBegin = 0;
    static int editMinuteBegin = 0;
    static int editDayEnd = 1;
    static int editHourEnd = 0;
    static int editMinuteEnd = 0;

    if (ImGui::BeginListBox("Text Presets")) {
        std::vector<TrailMask>* trailMasks = appState->trailMasks;
        size_t usedTrailMaskIndex = appState->usedTrailMaskIndex;

        if(trailMasks != nullptr) {
            for (unsigned int i = 0; i < trailMasks->size(); ++i) {
                TrailMask& trailMask = (*trailMasks)[i];

                if(!trailMask.isText) {
                    continue;
                }

                ImGui::PushID(static_cast<int>(i));

                //display clock symbol (0xF017) if entry has a time slot
				const bool isSelected = (usedTrailMaskIndex == i);
				const std::string displayName = trailMask.imageName +
					(trailMask.hasTimeSlot ? "  " : "");

				if(ImGui::Selectable(displayName.c_str(), isSelected)) {
					appState->usedTrailMaskIndex = i;
					notify(UserEvent{EventType::LOAD_NEW_PICTURE, 0, 0});
					std::cout << "Selected Text: " << trailMask.imageName << std::endl;
				}

                if(ImGui::BeginPopupContextItem("TextPresetContext")) {
                    if(ImGui::IsWindowAppearing()) {
                        isEditingTextPreset_ = true;
                        textToEdit_ = trailMask.imageName;

                        editTimeSlot = trailMask.hasTimeSlot;
                        editDayBegin = 1;
                        editHourBegin = 0;
                        editMinuteBegin = 0;
                        editDayEnd = 1;
                        editHourEnd = 0;
                        editMinuteEnd = 0;

                        SDL_DateTime beginDateTime{};
                        SDL_DateTime endDateTime{};

                        if(trailMask.hasTimeSlot &&
                           SDL_TimeToDateTime(trailMask.beginTimeSlot, &beginDateTime, true) &&
                           SDL_TimeToDateTime(trailMask.endTimeSlot, &endDateTime, true)) {
                            editDayBegin = static_cast<int>(beginDateTime.day);
                            editHourBegin = static_cast<int>(beginDateTime.hour);
                            editMinuteBegin = static_cast<int>(beginDateTime.minute);
                            editDayEnd = static_cast<int>(endDateTime.day);
                            editHourEnd = static_cast<int>(endDateTime.hour);
                            editMinuteEnd = static_cast<int>(endDateTime.minute);
                        }
                    }

                    ImGui::Text("Edit text:");
                    ImGui::InputTextMultiline(
                        "##edittext",
                        &textToEdit_,
                        ImVec2(-FLT_MIN, ImGui::GetTextLineHeight() * 8),
                        ImGuiInputTextFlags_CallbackCharFilter,
                        TextFilters::FilterASCII
                    );

                    ImGui::Separator();
                    ImGui::Checkbox("Time Slot", &editTimeSlot);

                    if(editTimeSlot) {
                        ImGui::Text("Begin:");
                        ImGui::DragInt("Day Begin", &editDayBegin, 1, 1, 31);
                        ImGui::DragInt("Hour Begin", &editHourBegin, 1, 0, 23);
                        ImGui::DragInt("Minute Begin", &editMinuteBegin, 1, 0, 59);

                        ImGui::Text("End:");
                        ImGui::DragInt("Day End", &editDayEnd, 1, 1, 31);
                        ImGui::DragInt("Hour End", &editHourEnd, 1, 0, 23);
                        ImGui::DragInt("Minute End", &editMinuteEnd, 1, 0, 59);
                    }

                    ImGui::Separator();

                    if(ImGui::Button("OK")) {
                        TrailMaskData trailMaskData{
                            .newName = textToEdit_,
                            .isText = true,
                            .hasTimeSlot = editTimeSlot,
                            .dayBegin = editDayBegin,
                            .hourBegin = editHourBegin,
                            .minuteBegin = editMinuteBegin,
                            .dayEnd = editDayEnd,
                            .hourEnd = editHourEnd,
                            .minuteEnd = editMinuteEnd
                        };

                        notify(UserEvent{
                            EventType::EDIT_TEXT_TEXTURE,
                            static_cast<int>(i),
                            trailMaskData
                        });

                        textToEdit_.clear();
                        isEditingTextPreset_ = false;
                        ImGui::CloseCurrentPopup();
                    }

                    ImGui::SameLine();

                    if(ImGui::Button("Cancel")) {
                        textToEdit_.clear();
                        isEditingTextPreset_ = false;
                        ImGui::CloseCurrentPopup();
                    }

                    ImGui::SameLine();

                    if(ImGui::Button("Delete")) {
                        notify(UserEvent{
                            EventType::DELETE_TEXT_TEXTURE,
                            static_cast<int>(i),
                            textToEdit_
                        });

                        textToEdit_.clear();
                        isEditingTextPreset_ = false;
                        ImGui::CloseCurrentPopup();
                    }

                    ImGui::EndPopup();
                }

                if(isSelected) {
                    ImGui::SetItemDefaultFocus();
                }

                ImGui::SetItemTooltip("Right-click to Edit or Delete");
                ImGui::PopID();
            }
        }

        ImGui::EndListBox();
    }

    ImGui::SameLine();
    if(ImGui::Button("New Text")) {
        ImGui::OpenPopup("New Text");
    }

    if(ImGui::BeginPopupModal("New Text", NULL, ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::Text("Enter new text:");
        static std::string newTextBuffer = "";

        ImGui::InputTextMultiline(
            "##newtext",
            &newTextBuffer,
            ImVec2(-FLT_MIN, ImGui::GetTextLineHeight() * 8),
            ImGuiInputTextFlags_CallbackCharFilter,
            TextFilters::FilterASCII
        );

        ImGui::Separator();

        if(ImGui::Button("OK")) {
            notify(UserEvent{EventType::CREATE_NEW_TEXT_TEXTURE, newTextBuffer, 0});
            newTextBuffer.clear();
            ImGui::CloseCurrentPopup();
        }

        ImGui::SameLine();

        if(ImGui::Button("Cancel")) {
            newTextBuffer.clear();
            ImGui::CloseCurrentPopup();
        }

        ImGui::EndPopup();
    }

        ImGui::SliderFloat(
        "Trail Mask Influence",
        &appState->universalShaderSettings.trailMaskInfluence,
        0.0f,
        5.0f
    );

    if(ImGui::SliderFloat(
        "Trail Mask Scale X",
        &appState->universalShaderSettings.trailMaskScaleX,
        0.1f,
        10.0f
    ) && linkTrailMaskScales_) {
        appState->universalShaderSettings.trailMaskScaleY =
            appState->universalShaderSettings.trailMaskScaleX;
    }

    if(ImGui::SliderFloat(
        "Trail Mask Scale Y",
        &appState->universalShaderSettings.trailMaskScaleY,
        0.1f,
        10.0f
    ) && linkTrailMaskScales_) {
        appState->universalShaderSettings.trailMaskScaleX =
            appState->universalShaderSettings.trailMaskScaleY;
    }

    if(ImGui::link("Link##first_second", &linkTrailMaskScales_, 2, 2.0f)) {
        appState->universalShaderSettings.trailMaskScaleY =
        appState->universalShaderSettings.trailMaskScaleX;
    }

    ImGui::SliderFloat(
        "Trail Mask Position X",
        &appState->universalShaderSettings.trailMaskPosition.x,
        -1.0f,
        1.0f
    );
    ImGui::SliderFloat(
        "Trail Mask Position Y",
        &appState->universalShaderSettings.trailMaskPosition.y,
        -1.0f,
        1.0f
    );
    ImGui::SliderInt(
        "Trail Mask Time Intervall [s]",
        &appState->trailMaskIntervall,
        2,
        60
    );
}