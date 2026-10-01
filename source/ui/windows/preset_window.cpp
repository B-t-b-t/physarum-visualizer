#include "preset_window.h"

#include "helper/preset_window_helper.h"
#include "../../preset_system.h"
#include "../../preset_types.h"  // for BehaviorPreset, ColorPreset
#include "../../simulation/trail_map_controller.h"
#include "../../utility/event.h"
#include "../widgets/link_widget.h"
#include "../widgets/color_indicator.h"
#include "../../../external/imgui_stdlib.h"

void PresetWindow::render(ApplicationState* appState) {
    if(!visible) { return; }

	ImGui::Begin("Preset", &visible);

	ImGui::PushItemWidth(widgetWidth);

	if (ImGui::BeginTabBar("Preset Types")) {
        //Fontawesome: fa-solid fa-barcode 
		if (ImGui::BeginTabItem("\uf02a Behaviour")) {
			behaviourPresetGUI(appState);
			ImGui::EndTabItem();
		}
        //Fontawesome: fa-solid fa-palette 
		if (ImGui::BeginTabItem("\uf53f Color")) {
			colorPresetGUI(appState);
			ImGui::EndTabItem();
		}
        //Fontawesome: fa-solid fa-images 
		if (ImGui::BeginTabItem("\uf302 Image")) {
			imagePresetGUI(appState);
			ImGui::EndTabItem();
		}
        //Fontawesome: fa-solid fa-font 
		if (ImGui::BeginTabItem("\uf031 Text")) {
			textPresetGUI(appState);
			ImGui::EndTabItem();
		}
		ImGui::EndTabBar();
	}
	ImGui::SeparatorText("Shared Options");
	ImGui::Checkbox("Auto Preset Switching", &appState->autoPresetSwitching);
	ImGui::SliderInt("Switch at Beat Volume", &appState->beatVolumeSwitch, 0, 50);

	ImGui::PopItemWidth();
	ImGui::End();
}

void PresetWindow::behaviourPresetGUI(ApplicationState* appState) {
	//--------------------------------
	//Preset System
	//--------------------------------
    ImGui::SeparatorText("Selection");

    ImGui::SetNextWindowSizeConstraints(ImVec2(ImGui::GetContentRegionAvail().x * 0.5f, ImGui::GetTextLineHeightWithSpacing() * 1), ImVec2(ImGui::GetContentRegionAvail().x * 0.5f, ImGui::GetTextLineHeightWithSpacing() * 8));

    ImGui::BeginChild("BehaviorSelectionChild", ImVec2(0, 0), true, ImGuiWindowFlags_MenuBar);

    std::map<std::string, BehaviorPreset>* behaviorPresets = appState->behaviorPresets;

    if (ImGui::BeginMenuBar()) {
        //Fontawesome: fa-solid fa-square-plus 
        if(ImGui::MenuItem("\uf0fe Add")) {
            ImGui::OpenPopup("New Behavior Preset");
        }
        PresetWindowHelper::behaviorPresetAddModal("New Behavior Preset", appState, this);
        
        //only show when there is data to show or delete
        if(behaviorPresets != nullptr && !behaviorPresets->empty()) {
            //Fontawesome: fa-solid fa-pen-to-square 
            if(ImGui::MenuItem("\uf044 Edit")) {
                ImGui::OpenPopup("Edit Behavior Preset");
            }
            //PresetWindowHelper::behaviorPresetEditModal("Edit Behavior Preset", appState, this);
            //Fontawesome: fa-solid fa-trash-can 
            if(ImGui::MenuItem("\uf2ed Delete")) {
                ImGui::OpenPopup("Delete Behavior Preset");
            }
            PresetWindowHelper::behaviorPresetDeleteModal("Delete Behavior Preset", appState, this);
        }
        ImGui::EndMenuBar();
    }

    if(ImGui::BeginTable("##Behavior Selection", 1, ImGuiTableFlags_SizingFixedFit)) {
		for (const auto& [presetName, preset] : *behaviorPresets) {
            ImGui::TableNextRow();
            ImGui::TableNextColumn();
			const bool is_selected = (presetName == appState->usedBehaviorPresetName);
			if (ImGui::Selectable(presetName.c_str(), is_selected, ImGuiSelectableFlags_SpanAllColumns)) {
				appState->usedBehaviorPresetName = presetName;
				notify(UserEvent{EventType::BEHAVIOR_PRESET_APPLY, presetName});
				std::cout << "Selected Preset: " << presetName << std::endl;
			}
		}

		ImGui::EndTable();
	}

    ImGui::EndChild();

    ImGui::SeparatorText("Settings");

	ImGui::SliderInt("Switch Intervall", &appState->presetIntervall, 2, 60, "%d s");
}

void PresetWindow::colorPresetGUI(ApplicationState* appState) {
	//--------------------------------
	//Color Preset System
	//--------------------------------

    ImGui::SeparatorText("Selection");

    ImGui::SetNextWindowSizeConstraints(ImVec2(ImGui::GetContentRegionAvail().x * 0.5f, ImGui::GetTextLineHeightWithSpacing() * 1), ImVec2(ImGui::GetContentRegionAvail().x * 0.5f, ImGui::GetTextLineHeightWithSpacing() * 8));

    ImGui::BeginChild("ColorSelectionChild", ImVec2(0, 0), true, ImGuiWindowFlags_MenuBar);

    std::map<std::string, ColorPreset>* colorPresets = appState->colorPresets;

    if (ImGui::BeginMenuBar()) {
        //Fontawesome: fa-solid fa-square-plus 
        if(ImGui::MenuItem("\uf0fe Add")) {
            ImGui::OpenPopup("New Color Preset");
        }
        PresetWindowHelper::colorPresetAddModal("New Color Preset", appState, this);

        //only show when there is data to show or delete
        if(colorPresets != nullptr && !colorPresets->empty()) {
            //Fontawesome: fa-solid fa-pen-to-square 
            if(ImGui::MenuItem("\uf044 Edit")) {
                ImGui::OpenPopup("Edit Color Preset");
            }
            //PresetWindowHelper::colorPresetEditModal("Edit Color Preset", appState, this);
            //Fontawesome: fa-solid fa-trash-can 
            if(ImGui::MenuItem("\uf2ed Delete")) {
                ImGui::OpenPopup("Delete Color Preset");
            }
            PresetWindowHelper::colorPresetDeleteModal("Delete Color Preset", appState, this);
        }
        
        ImGui::EndMenuBar();
    }

    if(ImGui::BeginTable("##Color Selection", 2, ImGuiTableFlags_SizingFixedFit)) {

        for (const auto& [presetName, preset] : *colorPresets) {
            ImGui::TableNextRow();
            ImGui::TableNextColumn();
            const bool is_selected = (presetName == appState->usedColorPresetName);
            ImGui::SetNextItemAllowOverlap();
            if (ImGui::Selectable(presetName.c_str(), is_selected, ImGuiSelectableFlags_SpanAllColumns)) {
                appState->usedColorPresetName = presetName;
                //appState->loadFromColorPreset = true;
                notify(UserEvent{EventType::COLOR_PRESET_APPLY, presetName});
                std::cout << "Selected Color Preset: " << presetName << std::endl;
            }

            ImGui::TableNextColumn();

            //crude conversion between phys::Vec4 and ImVec4, sufficient for now...
            ImVec4 color0 = ImVec4(preset.slimeColor0.x, preset.slimeColor0.y, preset.slimeColor0.z, preset.slimeColor0.w);
            ImVec4 color1 = ImVec4(preset.slimeColor1.x, preset.slimeColor1.y, preset.slimeColor1.z, preset.slimeColor1.w);
            ImVec4 color2 = ImVec4(preset.slimeColor2.x, preset.slimeColor2.y, preset.slimeColor2.z, preset.slimeColor2.w);

            ImGui::ColorIndicator({color0, color1, color2}, 0.5f);
        }

        ImGui::EndTable();
    }

    ImGui::EndChild();

    ImGui::SeparatorText("Settings");

	ImGui::SliderInt("Switch Intervall", &appState->colorPresetIntervall, 2, 60, "%d s");
}

void PresetWindow::imagePresetGUI(ApplicationState* appState) {
	//--------------------------------
    //Picture Selection
    //--------------------------------

    ImGui::Text("Influence the Trail with an Image:");
    ImGui::SeparatorText("Selection");

    ImGui::SetNextWindowSizeConstraints(ImVec2(ImGui::GetContentRegionAvail().x * 0.5f, ImGui::GetTextLineHeightWithSpacing() * 1), ImVec2(ImGui::GetContentRegionAvail().x * 0.5f, ImGui::GetTextLineHeightWithSpacing() * 8));

    ImGui::BeginChild("ImageSelectionChild", ImVec2(0, 0), true, ImGuiWindowFlags_MenuBar);

    std::map<std::string, TrailMask>* trailMasks = appState->trailMasks;
    const std::string& usedTrailMaskName = appState->usedTrailMaskName;

    if (ImGui::BeginMenuBar()) {
        //Fontawesome: fa-solid fa-square-plus 
        if(ImGui::MenuItem("\uf0fe Add")) {
            ImGui::OpenPopup("New Image Preset");
        }
        //PresetWindowHelper::imagePresetAddModal("New Image Preset", appState, this);

        //only show when there is data to show or delete
        if(trailMasks != nullptr && !trailMasks->empty()) {
            //Fontawesome: fa-solid fa-pen-to-square 
            if(ImGui::MenuItem("\uf044 Edit")) {
                ImGui::OpenPopup("Edit Image Preset");
            }
            //PresetWindowHelper::imagePresetEditModal("Edit Image Preset", (*trailMasks)[usedTrailMaskIndex], this, &usedTrailMaskIndex);

            //Fontawesome: fa-solid fa-trash-can 
            if(ImGui::MenuItem("\uf2ed Delete")) {
                ImGui::OpenPopup("Delete Image Preset");
            }
            //PresetWindowHelper::imagePresetDeleteModal("Delete Image Preset", appState, this);
        }
        
        ImGui::EndMenuBar();
    }

    if(ImGui::BeginTable("##Image Selection", 2, ImGuiTableFlags_SizingFixedFit)) {

        if(trailMasks != nullptr) {
            static std::string editKey = "";
            static bool isRightClicked = false;

            for(auto& [key, trailMask] : *trailMasks) {

                if(trailMask.type != TrailMaskType::TEXT) {
                    ImGui::TableNextRow();
                    ImGui::TableNextColumn();
                } else {
                    continue;       //filter out text presets
                }

				const bool isSelected = (usedTrailMaskName == key);

				if(ImGui::Selectable(trailMask.name.c_str(), isSelected, ImGuiSelectableFlags_SpanAllColumns)) {
					appState->usedTrailMaskName = key;
					notify(UserEvent{EventType::IMAGE_PRESET_APPLY});
					std::cout << "Selected Image: " << trailMask.name << std::endl;
				}

                ImGui::SetItemTooltip("Right-click to edit");

                if(ImGui::OpenPopupOnItemClick(nullptr, ImGuiPopupFlags_MouseButtonRight)) {
                    editKey = key;
                    isRightClicked = true;
                }

                ImGui::TableNextColumn();
                //Fontawesome: fa-solid fa-clock 
                const std::string infoString = trailMask.timeSlot ? "\uf017" : "";
                ImGui::Text("%s", infoString.c_str());
            }

            if(isRightClicked) {
                ImGui::OpenPopup("Edit Image");
                isRightClicked = false;
            }
            PresetWindowHelper::imagePresetEditModal("Edit Image", (*trailMasks).find(editKey)->second, this);   
        }

        ImGui::EndTable();
    }

    ImGui::EndChild();

    ImGui::SeparatorText("Settings");

    if(ImGui::SliderFloat("Strength", &appState->universalShaderSettings.trailMaskInfluence, 0.0f, 5.0f)) {
        notify(UserEvent{EventType::TRAIL_MASK_STRENGTH_CHANGED, appState->universalShaderSettings.trailMaskInfluence});
    }

    ImGui::LinkBegin("##Link Trail Mask Scales", &linkTrailMaskScales_, 2.0f);

    ImGui::LinkSliderFloat("Width", &appState->universalShaderSettings.trailMaskScaleX, 0.1f, 10.0f);
    ImGui::LinkSliderFloat("Height", &appState->universalShaderSettings.trailMaskScaleY, 0.1f, 10.0f);

    ImGui::LinkEnd();

    ImGui::SliderFloat("Position Horizontal", &appState->universalShaderSettings.trailMaskPosition.x, -1.0f, 1.0f);
    ImGui::SliderFloat("Position Vertical", &appState->universalShaderSettings.trailMaskPosition.y, -1.0f, 1.0f);
    ImGui::SliderInt("Switch Intervall", &appState->trailMaskIntervall, 2, 60, "%d s");
    ImGui::SliderInt("Disable at Sensor Distance", &appState->disableAtSensorDistance, 1, 100, "≥%d");
}

void PresetWindow::textPresetGUI(ApplicationState* appState) {
    //--------------------------------
    //Text Selection
    //--------------------------------

    ImGui::Text("Influence the Trail with a Text:");
    ImGui::SeparatorText("Selection");

    ImGui::SetNextWindowSizeConstraints(ImVec2(ImGui::GetContentRegionAvail().x * 0.5f, ImGui::GetTextLineHeightWithSpacing() * 1), ImVec2(ImGui::GetContentRegionAvail().x * 0.5f, ImGui::GetTextLineHeightWithSpacing() * 8));

    ImGui::BeginChild("TextSelectionChild", ImVec2(0, 0), true, ImGuiWindowFlags_MenuBar);

    std::string usedTrailMaskName = appState->usedTrailMaskName;
    std::map<std::string, TrailMask>* trailMasks = appState->trailMasks;

    if (ImGui::BeginMenuBar()) {
        //Fontawesome: fa-solid fa-square-plus 
        if(ImGui::MenuItem("\uf0fe Add")) {
            ImGui::OpenPopup("New Text Preset");
        }
        PresetWindowHelper::textPresetAddModal("New Text Preset", this);

        //only show when there is data to show or delete
        if(trailMasks != nullptr && !trailMasks->empty()) {
            //Fontawesome: fa-solid fa-pen-to-square 
            if(ImGui::MenuItem("\uf044 Edit")) {    //edit a already selected text preset
                ImGui::OpenPopup("Edit Text Preset");
            }
            PresetWindowHelper::textPresetEditModal("Edit Text Preset", (*trailMasks).find(usedTrailMaskName)->second, this);

            //Fontawesome: fa-solid fa-trash-can 
            if(ImGui::MenuItem("\uf2ed Delete")) {
                ImGui::OpenPopup("Delete Text Preset");
            }
            PresetWindowHelper::textPresetDeleteModal("Delete Text Preset", (*trailMasks).find(usedTrailMaskName)->second, this);
        }
        ImGui::EndMenuBar();
    }

    if(ImGui::BeginTable("##Text Selection", 2, ImGuiTableFlags_SizingFixedFit)) {
        if(trailMasks != nullptr) {

            static std::string editKey = "";
            static bool isRightClicked = false;

            for (auto& [key, trailMask] : *trailMasks) {
                
                if(trailMask.type == TrailMaskType::TEXT) {
                    ImGui::TableNextRow();
                    ImGui::TableNextColumn();
                } else {
                    continue;   //filter out non-text presets
                }

				const bool isSelected = (usedTrailMaskName == key);
				if(ImGui::Selectable(trailMask.name.c_str(), isSelected, ImGuiSelectableFlags_SpanAllColumns)) {
					appState->usedTrailMaskName = key;
					notify(UserEvent{EventType::TEXT_PRESET_APPLY});
					std::cout << "Selected Text: " << trailMask.name << std::endl;
				}

                if(ImGui::OpenPopupOnItemClick(nullptr, ImGuiPopupFlags_MouseButtonRight)) {
                    editKey = key;
                    isRightClicked = true;
                }

                ImGui::SetItemTooltip("Right-click to Edit");

                ImGui::TableNextColumn();
                //Fontawesome: fa-solid fa-clock 
                const std::string infoString = trailMask.timeSlot ? "\uf017" : "";
                ImGui::Text("%s", infoString.c_str());
            }

            if(isRightClicked) {
                ImGui::OpenPopup("Edit Text");  //edit a non-selected text preset
                isRightClicked = false;
            }
            PresetWindowHelper::textPresetEditModal("Edit Text", (*trailMasks).find(editKey)->second, this);
        }

        ImGui::EndTable();
    }

    ImGui::EndChild();

    ImGui::SeparatorText("Settings");

    ImGui::SliderInt("Switch Intervall", &appState->trailMaskIntervall, 2, 60, "%d s");
    ImGui::SliderInt("Disable at Sensor Distance", &appState->disableAtSensorDistance, 1, 100, "≥%d");
}