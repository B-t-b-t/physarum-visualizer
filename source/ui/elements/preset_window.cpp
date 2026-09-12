#include "preset_window.h"

#include "../../utility/event.h"
#include "../../simulation/trail_map_controller.h"
#include "misc/cpp/imgui_stdlib.cpp"	//for string parameters in text input fields
#include "link_element.h"

void PresetWindow::render(ApplicationState* appState) {
    if(!visible) { return; }

	ImGui::Begin("Preset", &visible);

	if (ImGui::BeginTabBar("Preset Types")) {
        //Fontawesome: Barcode Icon (Unicode: 0xF02A)
		if (ImGui::BeginTabItem(" Behaviour")) {
			behaviourPresetGUI(appState);
			ImGui::EndTabItem();
		}
        //Fontawesome: Palette Icon (Unicode: 0xF53F)
		if (ImGui::BeginTabItem(" Color")) {
			colorPresetGUI(appState);
			ImGui::EndTabItem();
		}
        //Fontawesome: Images Icon (Unicode: 0xF302)
		if (ImGui::BeginTabItem(" Image")) {
			imagePresetGUI(appState);
			ImGui::EndTabItem();
		}
        //Fontawesome: Font Icon (Unicode: 0xF031)
		if (ImGui::BeginTabItem(" Text")) {
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
    ImGui::SeparatorText("Selection");
	if (ImGui::BeginListBox("##Behavior Selection")) {

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

    ImGui::SameLine();
    //Fontawesome Plus Symbol (Unicode: 0xF0FE)
    if(ImGui::Button(" Add")) {
        ImGui::OpenPopup("New Behavior");
    }

    if(ImGui::BeginPopupModal("New Behavior", NULL, ImGuiWindowFlags_AlwaysAutoResize)) {
        
        static char presetNameChar[128] = "";
        ImGui::InputTextWithHint("##New Behavior Name", "Name", presetNameChar, IM_ARRAYSIZE(presetNameChar), ImGuiInputTextFlags_CharsNoBlank);

        static bool presetAlreadyExists = false;

        ImGui::Separator();

        if (ImGui::Button("Save") && presetNameChar[0] != '\0') {

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
            ImGui::CloseCurrentPopup();
        }

        ImGui::SameLine();

        if(ImGui::Button("Cancel")) {
            presetNameChar[0] = '\0';
            ImGui::CloseCurrentPopup();
        }

        if(presetAlreadyExists && ImGui::IsItemHovered()){
            ImGui::Text("Behavior with this name already exists!");
        } else {
            presetAlreadyExists = false;
        }

        ImGui::EndPopup();
    }

    ImGui::SeparatorText("Settings");

	ImGui::SliderInt("Switch Intervall [s]", &appState->presetIntervall, 2, 60);
}

void PresetWindow::colorPresetGUI(ApplicationState* appState) {
	//--------------------------------
	//Color Preset System
	//--------------------------------
    ImGui::SeparatorText("Selection");

	if (ImGui::BeginListBox("##Color Selection")) {

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

        ImGui::SameLine();
    //Fontawesome Plus Symbol (Unicode: 0xF0FE)
    if(ImGui::Button(" Add")) {
        ImGui::OpenPopup("New Color");
    }

    if(ImGui::BeginPopupModal("New Color", NULL, ImGuiWindowFlags_AlwaysAutoResize)) {
        
        static char colorPresetNameChar[128] = "";
        ImGui::InputTextWithHint("##New Color Name", "Name", colorPresetNameChar, IM_ARRAYSIZE(colorPresetNameChar), ImGuiInputTextFlags_CharsNoBlank);

        static bool presetAlreadyExists = false;

        ImGui::Separator();

        if (ImGui::Button("Save") && colorPresetNameChar[0] != '\0') {

            for(unsigned int i = 0; i < colorPresetNames_.size(); i++) {
                if(colorPresetNames_[i] == std::string(colorPresetNameChar)) {
                    presetAlreadyExists = true;
                }
            }

            if(!presetAlreadyExists) {
                addColorPresetName(std::string(colorPresetNameChar));
                notify(UserEvent{EventType::SAVE_COLOR_PRESET, 0, 0});
                //appState->saveToColorPreset = true;
            }

            colorPresetNameChar[0] = '\0';
            ImGui::CloseCurrentPopup();
        }

        ImGui::SameLine();

        if(ImGui::Button("Cancel")) {
            colorPresetNameChar[0] = '\0';
            ImGui::CloseCurrentPopup();
        }

        if(presetAlreadyExists && ImGui::IsItemHovered()){
            ImGui::Text("Color with this name already exists!");
        } else {
            presetAlreadyExists = false;
        }

        ImGui::EndPopup();
    }

    ImGui::SeparatorText("Settings");

	ImGui::SliderInt("Switch Intervall [s]", &appState->colorPresetIntervall, 2, 60);
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

    ImGui::Text("Influence the Trail with an Image:");
    ImGui::SeparatorText("Selection");

    if(ImGui::BeginListBox("##Images")) {
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
					std::cout << "Selected Image: " << trailMask.imageName << std::endl;
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
                    ImGui::Text("Edit Image: %s", trailMask.imageName.c_str());
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

                    if(ImGui::Button("Ok")) {
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

                ImGui::SetItemTooltip("Right-click to edit");
                ImGui::PopID();
            }
        }

        ImGui::EndListBox();
    }

    ImGui::SeparatorText("Settings");

    if(ImGui::SliderFloat(
        "Strength",
        &appState->universalShaderSettings.trailMaskInfluence,
        0.0f,
        5.0f
    )) {
        notify(UserEvent{
            EventType::TRAIL_MASK_STRENGTH_CHANGED,
            0,
            appState->universalShaderSettings.trailMaskInfluence
        });
    }

    ImGui::LinkBegin("##Link Trail Mask Scales", &linkTrailMaskScales_, 2.0f);

    ImGui::LinkSliderFloat(
        "Width",
        &appState->universalShaderSettings.trailMaskScaleX,
        0.1f,
        10.0f
    );
    
    ImGui::LinkSliderFloat(
        "Height",
        &appState->universalShaderSettings.trailMaskScaleY,
        0.1f,
        10.0f
    );

    ImGui::LinkEnd();

    ImGui::SliderFloat(
        "Position Horizontal",
        &appState->universalShaderSettings.trailMaskPosition.x,
        -1.0f,
        1.0f
    );
    ImGui::SliderFloat(
        "Position Vertical",
        &appState->universalShaderSettings.trailMaskPosition.y,
        -1.0f,
        1.0f
    );
    ImGui::SliderInt(
        "Switch Intervall [s]",
        &appState->trailMaskIntervall,
        2,
        60
    );

    ImGui::SliderInt(
        "Disable at Sensor Distance >=",
        &appState->disableAtSensorDistance,
        1,
        100
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

    ImGui::Text("Influence the Trail with a Text:");
    ImGui::SeparatorText("Selection");

    
    if (ImGui::BeginListBox("##Text Presets")) {
        size_t usedTrailMaskIndex = appState->usedTrailMaskIndex;
        std::vector<TrailMask>* trailMasks = appState->trailMasks;

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

                    if(ImGui::Button("Ok") && !textToEdit_.empty()) {
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

                    //Fontawesome Trash Symbol (Unicode: 0xF2ED)
                    if(ImGui::Button(" Delete")) {
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

                ImGui::SetItemTooltip("Right-click to Edit");
                ImGui::PopID();
            }
        }

        ImGui::EndListBox();
    }

    ImGui::SameLine();
    //Fontawesome Plus Symbol (Unicode: 0xF0FE)
    if(ImGui::Button(" New")) {
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

        if(ImGui::Button("Ok") && !newTextBuffer.empty()) {
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

    ImGui::SeparatorText("Settings");

    if(ImGui::SliderFloat(
        "Strength",
        &appState->universalShaderSettings.trailMaskInfluence,
        0.0f,
        5.0f
    )) {
        notify(UserEvent{
            EventType::TRAIL_MASK_STRENGTH_CHANGED,
            0,
            appState->universalShaderSettings.trailMaskInfluence
        });
    }

    ImGui::LinkBegin("##Link Trail Mask Scales", &linkTrailMaskScales_, 2.0f);

    ImGui::LinkSliderFloat(
        "Width",
        &appState->universalShaderSettings.trailMaskScaleX,
        0.1f,
        10.0f
    );

    ImGui::LinkSliderFloat(
        "Height",
        &appState->universalShaderSettings.trailMaskScaleY,
        0.1f,
        10.0f
    );

    ImGui::LinkEnd();

    ImGui::SliderFloat(
        "Position Horizontal",
        &appState->universalShaderSettings.trailMaskPosition.x,
        -1.0f,
        1.0f
    );
    ImGui::SliderFloat(
        "Position Vertical",
        &appState->universalShaderSettings.trailMaskPosition.y,
        -1.0f,
        1.0f
    );
    ImGui::SliderInt(
        "Switch Intervall [s]",
        &appState->trailMaskIntervall,
        2,
        60
    );

    ImGui::SliderInt(
        "Disable at Sensor Distance >=",
        &appState->disableAtSensorDistance,
        1,
        100
    );
}