#include "preset_window_helper.h"

#include "imgui.h"

#include "../preset_window.h"
#include "../../widgets/color_indicator.h"
#include "../../../utility/event.h"
#include "../../../application_state.h"
#include "../../../preset_types.h"
#include "../../../simulation/trail_map_controller.h"
#include "../../../../external/imgui_stdlib.h"

namespace PresetWindowHelper {

    void behaviorPresetAddModal(const char* stringID, ApplicationState* appState, PresetWindow* presetWindow) {
        if(ImGui::BeginPopupModal(stringID, NULL, ImGuiWindowFlags_AlwaysAutoResize)) {
            
            static std::string newBehaviorName = "";
            ImGui::InputTextWithHint("##New Behavior Name", "Name", &newBehaviorName, ImGuiInputTextFlags_CharsNoBlank);

            static bool presetAlreadyExists = false;

            ImGui::Separator();

            if (ImGui::Button("Save") && !newBehaviorName.empty()) {

                if(appState->behaviorPresets->contains(newBehaviorName)) {
                    presetAlreadyExists = true;
                }

                if(!presetAlreadyExists) {
                    presetWindow->notify(UserEvent{EventType::BEHAVIOR_PRESET_CREATE, newBehaviorName, 0});
                }

                newBehaviorName.clear();
                ImGui::CloseCurrentPopup();
            }

            ImGui::SameLine();

            if(ImGui::Button("Cancel")) {
                newBehaviorName.clear();
                ImGui::CloseCurrentPopup();
            }

            if(presetAlreadyExists && ImGui::IsItemHovered()){
                ImGui::Text("Behavior with this name already exists!");
            } else {
                presetAlreadyExists = false;
            }

            ImGui::EndPopup();
        }
    }

    void behaviorPresetDeleteModal(const char* stringID, ApplicationState* appState, PresetWindow* presetWindow) {
        if(ImGui::BeginPopupModal(stringID, NULL, ImGuiWindowFlags_AlwaysAutoResize)) {
            
            ImGui::Text("Name:   %s", appState->usedBehaviorPresetName.c_str());

            ImGui::Separator();

            //Fontawesome: fa-solid fa-trash-can 
            if (ImGui::Button("\uf2ed Delete")) {
                presetWindow->notify(UserEvent{EventType::BEHAVIOR_PRESET_DELETE, appState->usedBehaviorPresetName, 0});
                ImGui::CloseCurrentPopup();
            }
            ImGui::SameLine();
            if(ImGui::Button("Cancel")) {
                ImGui::CloseCurrentPopup();
            }
            ImGui::EndPopup();
        }
    }

    void colorPresetAddModal(const char* stringID, ApplicationState* appState, PresetWindow* presetWindow) {
        if(ImGui::BeginPopupModal(stringID, NULL, ImGuiWindowFlags_AlwaysAutoResize)) {
            
            static std::string newColorName = "";
            ImGui::InputTextWithHint("##New Color Name", "Name", &newColorName, ImGuiInputTextFlags_CharsNoBlank);

            ImGui::Text(" Colors:");
            ImGui::SameLine();
            ImGui::ColorIndicator({appState->slimeSettings.slimeColor0, appState->slimeSettings.slimeColor1, appState->slimeSettings.slimeColor2}, 0.5f);

            static bool presetAlreadyExists = false;

            ImGui::Separator();
            //Fontawesome: fa-solid fa-square-plus 
            if (ImGui::Button("\uf0fe Add") && !newColorName.empty()) {

                if(appState->colorPresets->contains(newColorName)) {
                    presetAlreadyExists = true;
                }

                if(!presetAlreadyExists) {
                    presetWindow->notify(UserEvent{EventType::COLOR_PRESET_CREATE, newColorName, 0});
                }

                newColorName.clear();
                ImGui::CloseCurrentPopup();
            }

            ImGui::SameLine();

            if(ImGui::Button("Cancel")) {
                newColorName.clear();
                ImGui::CloseCurrentPopup();
            }

            if(presetAlreadyExists && ImGui::IsItemHovered()){
                ImGui::Text("Color with this name already exists!");
            } else {
                presetAlreadyExists = false;
            }

            ImGui::EndPopup();
        }
    }

    void colorPresetDeleteModal(const char* stringID, ApplicationState* appState, PresetWindow* presetWindow) {
        if(ImGui::BeginPopupModal(stringID, NULL, ImGuiWindowFlags_AlwaysAutoResize)) {
            
            ImGui::Text("Name:   %s", appState->usedColorPresetName.c_str());
            ImGui::Text("Colors:");
            ImGui::SameLine();
            const ColorPreset& preset = appState->colorPresets->at(appState->usedColorPresetName);
            ImGui::ColorIndicator({preset.slimeColor0, preset.slimeColor1, preset.slimeColor2}, 0.5f);

            ImGui::Separator();

            //Fontawesome: fa-solid fa-trash-can 
            if (ImGui::Button("\uf2ed Delete")) {
                presetWindow->notify(UserEvent{EventType::COLOR_PRESET_DELETE, appState->usedColorPresetName, 0});
                ImGui::CloseCurrentPopup();
            }
            ImGui::SameLine();
            if(ImGui::Button("Cancel")) {
                ImGui::CloseCurrentPopup();
            }
            ImGui::EndPopup();
        }
    }
    //ImagePresetContext
    void imagePresetEditModal(const char* stringID, TrailMask& trailMask, PresetWindow* presetWindow, size_t atIndex) {
        if(ImGui::BeginPopupModal(stringID)) {
            
            static bool editTimeSlot = false;
            static int editDayBegin = 1;
            static int editHourBegin = 0;
            static int editMinuteBegin = 0;
            static int editDayEnd = 1;
            static int editHourEnd = 0;
            static int editMinuteEnd = 0;

            if(ImGui::IsWindowAppearing()) {
                editTimeSlot = trailMask.hasTimeSlot;
                editDayBegin = 1;
                editHourBegin = 0;
                editMinuteBegin = 0;
                editDayEnd = 1;
                editHourEnd = 0;
                editMinuteEnd = 0;
        
                SDL_DateTime beginDateTime{};
                SDL_DateTime endDateTime{};
        
                if(trailMask.hasTimeSlot 
                    && SDL_TimeToDateTime(trailMask.beginTimeSlot, &beginDateTime, true) 
                    && SDL_TimeToDateTime(trailMask.endTimeSlot, &endDateTime, true)
                ) {
                    editDayBegin = static_cast<int>(beginDateTime.day);
                    editHourBegin = static_cast<int>(beginDateTime.hour);
                    editMinuteBegin = static_cast<int>(beginDateTime.minute);
                    editDayEnd = static_cast<int>(endDateTime.day);
                    editHourEnd = static_cast<int>(endDateTime.hour);
                    editMinuteEnd = static_cast<int>(endDateTime.minute);
                }
            }


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

                presetWindow->notify(UserEvent{ EventType::EDIT_TRAIL_MASK_TIME_SLOT, static_cast<int>(atIndex), trailMaskData});

                ImGui::CloseCurrentPopup();
            }

            ImGui::SameLine();

            if(ImGui::Button("Cancel")) {
                ImGui::CloseCurrentPopup();
            }

            ImGui::EndPopup();
        }
    }

    void textPresetAddModal(const char* stringID, PresetWindow* presetWindow) {
        if(ImGui::BeginPopupModal(stringID, NULL, ImGuiWindowFlags_AlwaysAutoResize)) {
            ImGui::Text("Enter new text:");
            static std::string newTextBuffer = "";

            ImGui::InputTextMultiline("##newtext", &newTextBuffer, ImVec2(-FLT_MIN, ImGui::GetTextLineHeight() * 8), ImGuiInputTextFlags_CallbackCharFilter, TextFilters::FilterASCII);

            ImGui::Separator();

            if(ImGui::Button("Ok") && !newTextBuffer.empty()) {
                presetWindow->notify(UserEvent{EventType::TEXT_PRESET_CREATE, newTextBuffer, 0});
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
    }

    void textPresetEditModal(const char* stringID, TrailMask& trailMask, PresetWindow* presetWindow, size_t* atIndex) {
        if(ImGui::BeginPopupModal(stringID)) {
	        static std::string textToEdit{""};
            static bool editTimeSlot = false;
            static int editDayBegin = 1;
            static int editHourBegin = 0;
            static int editMinuteBegin = 0;
            static int editDayEnd = 1;
            static int editHourEnd = 0;
            static int editMinuteEnd = 0;

            if(ImGui::IsWindowAppearing()) {
                textToEdit = trailMask.imageName;
                editTimeSlot = trailMask.hasTimeSlot;

                SDL_DateTime beginDateTime{};
                SDL_DateTime endDateTime{};

                if(trailMask.hasTimeSlot 
                    && SDL_TimeToDateTime(trailMask.beginTimeSlot, &beginDateTime, true) 
                    && SDL_TimeToDateTime(trailMask.endTimeSlot, &endDateTime, true)
                ) {
                    editDayBegin = static_cast<int>(beginDateTime.day);
                    editHourBegin = static_cast<int>(beginDateTime.hour);
                    editMinuteBegin = static_cast<int>(beginDateTime.minute);
                    editDayEnd = static_cast<int>(endDateTime.day);
                    editHourEnd = static_cast<int>(endDateTime.hour);
                    editMinuteEnd = static_cast<int>(endDateTime.minute);
                }
            }

            ImGui::Text("Edit text:");
            ImGui::InputTextMultiline("##edittext", &textToEdit, ImVec2(-FLT_MIN, ImGui::GetTextLineHeight() * 8), ImGuiInputTextFlags_CallbackCharFilter, TextFilters::FilterASCII);

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

            if(ImGui::Button("Ok") && !textToEdit.empty()) {
                TrailMaskData trailMaskData{
                    .newName = textToEdit,
                    .isText = true,
                    .hasTimeSlot = editTimeSlot,
                    .dayBegin = editDayBegin,
                    .hourBegin = editHourBegin,
                    .minuteBegin = editMinuteBegin,
                    .dayEnd = editDayEnd,
                    .hourEnd = editHourEnd,
                    .minuteEnd = editMinuteEnd
                };

                presetWindow->notify(UserEvent{EventType::TEXT_PRESET_EDIT, static_cast<int>(*atIndex), trailMaskData});

                textToEdit.clear();
                ImGui::CloseCurrentPopup();
            }

            ImGui::SameLine();

            if(ImGui::Button("Cancel")) {
                textToEdit.clear();
                ImGui::CloseCurrentPopup();
            }

            ImGui::SameLine();

            //Fontawesome: fa-solid fa-trash-can 
            if(ImGui::Button("\uf2ed Delete")) {
                presetWindow->notify(UserEvent{EventType::TEXT_PRESET_DELETE, static_cast<int>(*atIndex), 0});

                textToEdit.clear();
                *atIndex = 0; //prevent out of bounds index, if last item was deleted
                ImGui::CloseCurrentPopup();
            }

            ImGui::EndPopup();
        }
    }

    void textPresetDeleteModal(const char* stringID, TrailMask& trailMask, PresetWindow* presetWindow, size_t* atIndex) {
        if(ImGui::BeginPopupModal(stringID, NULL, ImGuiWindowFlags_AlwaysAutoResize)) {
            
            ImGui::Text("Name:   %s", trailMask.imageName.c_str());

            ImGui::Separator();

            //Fontawesome: fa-solid fa-trash-can 
            if (ImGui::Button("\uf2ed Delete")) {
                presetWindow->notify(UserEvent{EventType::TEXT_PRESET_DELETE, static_cast<int>(*atIndex), 0});
                ImGui::CloseCurrentPopup();
            }
            ImGui::SameLine();
            if(ImGui::Button("Cancel")) {
                ImGui::CloseCurrentPopup();
            }
            ImGui::EndPopup();
        }
    }
}