#include "preset_window_helper.h"

#include <chrono>

#include "imgui.h"

#include "../preset_window.h"
#include "../../widgets/color_indicator.h"
#include "../../widgets/link_widget.h"
#include "../../widgets/time_picker.h"
#include "../../../utility/event.h"
#include "../../../utility/vector_math.h"
#include "../../../application_state.h"
#include "../../../preset_types.h"
#include "../../../simulation/trail_map_controller.h"
#include "../../../simulation/trail_mask_properties.h"
#include "../../../../external/imgui_stdlib.h"

namespace PresetWindowHelper {

    void behaviorPresetAddModal(const char* stringID, ApplicationState* appState, PresetWindow* presetWindow) {
        if(ImGui::BeginPopupModal(stringID, NULL, ImGuiWindowFlags_AlwaysAutoResize)) {
            
            static std::string newBehaviorName = "";
            static bool presetAlreadyExists = false;
            static bool isNameEmpty = false;

            if(ImGui::IsWindowAppearing()) {
                newBehaviorName.clear();
                presetAlreadyExists = false;
                isNameEmpty = false;
            }

            ImGui::InputTextWithHint("##New Behavior Name", "Name", &newBehaviorName, ImGuiInputTextFlags_CharsNoBlank);

            ImGui::Separator();
            if(presetAlreadyExists) {
                ImGui::TextColored(ImVec4(0.9f, 0.0f, 0.0f, 1.0f), "Behavior with this name already exists!");
            }
            if(isNameEmpty) {
                ImGui::TextColored(ImVec4(0.9f, 0.0f, 0.0f, 1.0f), "Name cannot be empty!");
            }

            if (ImGui::Button("Save")) {
                    presetAlreadyExists = appState->behaviorPresets->contains(newBehaviorName);
                    isNameEmpty = newBehaviorName.empty();

                if(!presetAlreadyExists && !isNameEmpty) {
                    presetWindow->notify(UserEvent{EventType::BEHAVIOR_PRESET_CREATE, newBehaviorName});
                    ImGui::CloseCurrentPopup();
                }
            }

            ImGui::SameLine();
            if(ImGui::Button("Cancel")) {
                ImGui::CloseCurrentPopup();
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
                presetWindow->notify(UserEvent{EventType::BEHAVIOR_PRESET_DELETE, appState->usedBehaviorPresetName});
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
            static bool presetAlreadyExists = false;
            static bool isNameEmpty = false;

            if(ImGui::IsWindowAppearing()) {
                newColorName.clear();
                presetAlreadyExists = false;
                isNameEmpty = false;
            }

            ImGui::InputTextWithHint("##New Color Name", "Name", &newColorName, ImGuiInputTextFlags_CharsNoBlank);

            ImGui::Text(" Colors:");
            ImGui::SameLine();

            //crude conversion between phys::Vec4 and ImVec4, sufficient for now...
            ImVec4 color0 = ImVec4(appState->slimeSettings.slimeColor0.x, appState->slimeSettings.slimeColor0.y, appState->slimeSettings.slimeColor0.z, appState->slimeSettings.slimeColor0.w);
            ImVec4 color1 = ImVec4(appState->slimeSettings.slimeColor1.x, appState->slimeSettings.slimeColor1.y, appState->slimeSettings.slimeColor1.z, appState->slimeSettings.slimeColor1.w);
            ImVec4 color2 = ImVec4(appState->slimeSettings.slimeColor2.x, appState->slimeSettings.slimeColor2.y, appState->slimeSettings.slimeColor2.z, appState->slimeSettings.slimeColor2.w);

            ImGui::ColorIndicator({color0, color1, color2}, 0.5f);

            ImGui::Separator();
            if(presetAlreadyExists) {
                ImGui::TextColored(ImVec4(0.9f, 0.0f, 0.0f, 1.0f), "Color with this name already exists!");
            }
            if(isNameEmpty) {
                ImGui::TextColored(ImVec4(0.9f, 0.0f, 0.0f, 1.0f), "Name cannot be empty!");
            }

            //Fontawesome: fa-solid fa-square-plus 
            if (ImGui::Button("\uf0fe Add")) {
                isNameEmpty = newColorName.empty();
                presetAlreadyExists = appState->colorPresets->contains(newColorName);

                if(!presetAlreadyExists && !isNameEmpty) {
                    presetWindow->notify(UserEvent{EventType::COLOR_PRESET_CREATE, newColorName});
                    ImGui::CloseCurrentPopup();
                }
            }

            ImGui::SameLine();
            if(ImGui::Button("Cancel")) {
                ImGui::CloseCurrentPopup();
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

            //crude conversion between phys::Vec4 and ImVec4, sufficient for now...
            ImVec4 color0 = ImVec4(preset.slimeColor0.x, preset.slimeColor0.y, preset.slimeColor0.z, preset.slimeColor0.w);
            ImVec4 color1 = ImVec4(preset.slimeColor1.x, preset.slimeColor1.y, preset.slimeColor1.z, preset.slimeColor1.w);
            ImVec4 color2 = ImVec4(preset.slimeColor2.x, preset.slimeColor2.y, preset.slimeColor2.z, preset.slimeColor2.w);

            ImGui::ColorIndicator({color0, color1, color2}, 0.5f);

            ImGui::Separator();

            //Fontawesome: fa-solid fa-trash-can 
            if (ImGui::Button("\uf2ed Delete")) {
                presetWindow->notify(UserEvent{EventType::COLOR_PRESET_DELETE, appState->usedColorPresetName});
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
    void imagePresetEditModal(const char* stringID, TrailMask& trailMask, PresetWindow* presetWindow) {
        using namespace std::chrono;

        if(ImGui::BeginPopupModal(stringID)) {
            
            static bool editTimeSlot = false;
            static float strength = 1.0f;
            static phys::Vec2 position{0.0f, 0.0f};
            static phys::Vec2 scale{1.0f, 1.0f};
            
            static int animationType{0};
            static phys::Vec2<double> positionVelocity{0.0, 0.0};
            static phys::Vec2 positionStart{0.0f, 0.0f};
            static phys::Vec2 positionEnd{0.0f, 0.0f};
            static double scaleVelocity{0.0};
            static float scaleStart{1.0f};
            static float scaleEnd{1.0f};

            static bool linkScale = true;
            static bool isInverted = false;

            //initialize in minutes for user convenience
            static system_clock::time_point timeSlotStart = floor<minutes>(system_clock::now());
            static system_clock::time_point timeSlotEnd = floor<minutes>(system_clock::now());

            if(ImGui::IsWindowAppearing()) {
                editTimeSlot = trailMask.timeSlot.has_value();
                strength = trailMask.strength;
                position = trailMask.position;
                scale = trailMask.scale;

                animationType = trailMask.animation.has_value();
                if(animationType) { animationType = trailMask.animation.value().hasTranslation() ? 1 : 2; }
                
                positionVelocity = animationType == 1 ? trailMask.animation.value().positionVelocity() : phys::Vec2<double>{0.0f, 0.0f};
                positionStart = animationType == 1 ? trailMask.animation.value().positionStart() : phys::Vec2{0.0f, 0.0f};
                positionEnd = animationType == 1 ? trailMask.animation.value().positionEnd() : phys::Vec2{0.0f, 0.0f};

                scaleVelocity = animationType == 2 ? trailMask.animation.value().scaleVelocity() : 0.0f;
                scaleStart = animationType == 2 ? trailMask.animation.value().scaleStart() : 1.0f;
                scaleEnd = animationType == 2 ? trailMask.animation.value().scaleEnd() : 1.0f;

                isInverted = trailMask.isInverted;

                //if no time slot exists, the start and end time shows the times of the last edited item by design
                //so that editing is more convenient for the user
                if(trailMask.timeSlot) {
                    timeSlotStart = trailMask.timeSlot.value().start;
                    timeSlotEnd = trailMask.timeSlot.value().end;
                }
            }


            ImGui::Text("Edit Image: %s", trailMask.name.c_str());
            ImGui::Separator();

            ImGui::Checkbox("Time Slot", &editTimeSlot);

            if(editTimeSlot) {
                editTimeSlotTable(&timeSlotStart, &timeSlotEnd);
            }

            ImGui::SliderFloat("Strength", &strength, 0.0f, 5.0f);
            ImGui::SliderFloat("Position Horizontal", &position.x, -1.0f, 1.0f);
            ImGui::SliderFloat("Position Vertical", &position.y, -1.0f, 1.0f);

            ImGui::LinkBegin("##Link Trail Mask Scales", &linkScale, 2.0f);
            ImGui::LinkSliderFloat("Width", &scale.x, 0.1f, 10.0f);
            ImGui::LinkSliderFloat("Height", &scale.y, 0.1f, 10.0f);
            ImGui::LinkEnd();

            ImGui::Text("Animation Type: ");
            ImGui::SameLine();
            ImGui::RadioButton("None", &animationType, 0);
            ImGui::SameLine();
            ImGui::RadioButton("Translation", &animationType, 1);
            ImGui::SameLine();
            ImGui::RadioButton("Scaling", &animationType, 2);

            switch (animationType) {
                case 0: { }  // None
                break;
                case 1: {   // Translation
                    ImGui::InputDouble("Position Horizontal Velocity", &positionVelocity.x, 0.001, 0.01);
                    ImGui::InputDouble("Position Vertical Velocity", &positionVelocity.y, 0.001, 0.01);
                    ImGui::SliderFloat("Start Position Horizontal", &positionStart.x, -1.0f, 1.0f);
                    ImGui::SliderFloat("Start Position Vertical", &positionStart.y, -1.0f, 1.0f);
                    ImGui::SliderFloat("End Position Horizontal", &positionEnd.x, -1.0f, 1.0f);
                    ImGui::SliderFloat("End Position Vertical", &positionEnd.y, -1.0f, 1.0f);
                }
                break;
                case 2: {   // Scaling
                    ImGui::InputDouble("Scale Velocity", &scaleVelocity, 0.01, 0.1);
                    ImGui::SliderFloat("Start Scale", &scaleStart, 0.1f, 10.0f);
                    ImGui::SliderFloat("End Scale", &scaleEnd, 0.1f, 10.0f);
                }
                break;
            }

            ImGui::Checkbox("Invert", &isInverted);

            ImGui::Separator();

            if(ImGui::Button("Ok")) {

                TrailMaskProperties properties{
                    .strength = strength,
                    .position = position,
                    .scale = scale,
                    .isInverted = isInverted
                };

                if(editTimeSlot) {
                    properties.timeSlot = TimeSlot{timeSlotStart, timeSlotEnd};
                }

                switch (animationType) {
                    case 0:
                        properties.animation = std::nullopt; // None
                        break;
                    case 1:
                        properties.animation = TrailMaskAnimation{positionVelocity, positionStart, positionEnd};
                        break;
                    case 2:
                        properties.animation = TrailMaskAnimation{scaleVelocity, scaleStart, scaleEnd};
                        break;
                }

                presetWindow->notify(UserEvent{ EventType::IMAGE_PRESET_EDIT, TrailMaskData{trailMask.name, TrailMaskType::IMAGE, properties}});
                presetWindow->notify(UserEvent{EventType::IMAGE_PRESET_APPLY});

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
        using namespace std::chrono;

        if(ImGui::BeginPopupModal(stringID, NULL, ImGuiWindowFlags_AlwaysAutoResize)) {
            static std::string name{""};
            static std::string text{""};
            static bool editTimeSlot = false;
            static TimeSlot timeSlot{};
            static bool timeSlotIsValid = true;
            static bool stringsEmpty = false;
            
            if(ImGui::IsWindowAppearing()) {
                name.clear();
                text.clear();
                editTimeSlot = false;
                //initialize in minutes for user convenience
                timeSlot = TimeSlot{floor<minutes>(system_clock::now()),floor<minutes>(system_clock::now())};
                timeSlotIsValid = true;
                stringsEmpty = false;
            }

            ImGui::Text("Enter preset name:");
            ImGui::InputText("##name", &name);
            ImGui::Text("Enter the text to display:");
            ImGui::InputTextMultiline("##newtext", &text, ImVec2(-FLT_MIN, ImGui::GetTextLineHeight() * 8), ImGuiInputTextFlags_CallbackCharFilter, TextFilters::FilterASCII);

            ImGui::Checkbox("Edit Time Slot", &editTimeSlot);
            if(editTimeSlot) {
                editTimeSlotTable(&timeSlot.start, &timeSlot.end);
            }

            ImGui::Separator();
            //error messages for user
            if(editTimeSlot && !timeSlotIsValid) {
                ImGui::TextColored(ImVec4(0.9f, 0.0f, 0.0f, 1.0f), "Timeslot duration must be longer than 0s!");
            }
            if(stringsEmpty) {
                ImGui::TextColored(ImVec4(0.9f, 0.0f, 0.0f, 1.0f), "Name or text cannot be empty!");
            }

            if(ImGui::Button("Ok")) {
                //data validity check
                timeSlotIsValid = !editTimeSlot || (editTimeSlot &&timeSlot.isValid());
                stringsEmpty = text.empty() || name.empty();

                if(timeSlotIsValid && !stringsEmpty) {
                    TrailMaskProperties properties{
                        .text = text,
                        .timeSlot = editTimeSlot ? std::optional<TimeSlot>{timeSlot} : std::nullopt
                    };

                    presetWindow->notify(UserEvent{EventType::TEXT_PRESET_CREATE, TrailMaskData{name, TrailMaskType::TEXT, properties}});
                    ImGui::CloseCurrentPopup();
                }
            }

            ImGui::SameLine();

            if(ImGui::Button("Cancel")) {
                ImGui::CloseCurrentPopup();
            }

            ImGui::EndPopup();
        }
    }

    void textPresetEditModal(const char* stringID, TrailMask& trailMask, PresetWindow* presetWindow) {
        using namespace std::chrono;

        if(ImGui::BeginPopupModal(stringID)) {
            static std::string name{""};
	        static std::string textToEdit{""};
            static bool editTimeSlot = false;
            static TimeSlot timeSlot{};
            static float strength = 1.0f;
            static phys::Vec2 position{0.0f, 0.0f};
            static phys::Vec2 scale{1.0f, 1.0f};
            static bool linkScale = true;
            static bool isInverted = false;
            static bool timeSlotIsValid = true;
            static bool stringsEmpty = false;

            static int animationType{0};
            static phys::Vec2<double> positionVelocity{0.0, 0.0};
            static phys::Vec2 positionStart{0.0f, 0.0f};
            static phys::Vec2 positionEnd{0.0f, 0.0f};
            static double scaleVelocity{0.0};
            static float scaleStart{1.0f};
            static float scaleEnd{1.0f};

            if(ImGui::IsWindowAppearing()) {
                name = trailMask.name;
                textToEdit = trailMask.text;
                editTimeSlot = trailMask.timeSlot.has_value();
                //initialize in minutes for user convenience
                timeSlot = TimeSlot(floor<minutes>(system_clock::now()), floor<minutes>(system_clock::now()));
                strength = trailMask.strength;
                position = trailMask.position;
                scale = trailMask.scale;
                isInverted = trailMask.isInverted;
                timeSlotIsValid = true;
                stringsEmpty = false;

                animationType = trailMask.animation.has_value();
                if(animationType) { animationType = trailMask.animation.value().hasTranslation() ? 1 : 2; }
                
                positionVelocity = animationType == 1 ? trailMask.animation.value().positionVelocity() : phys::Vec2<double>{0.0f, 0.0f};
                positionStart = animationType == 1 ? trailMask.animation.value().positionStart() : phys::Vec2{0.0f, 0.0f};
                positionEnd = animationType == 1 ? trailMask.animation.value().positionEnd() : phys::Vec2{0.0f, 0.0f};

                scaleVelocity = animationType == 2 ? trailMask.animation.value().scaleVelocity() : 0.0f;
                scaleStart = animationType == 2 ? trailMask.animation.value().scaleStart() : 1.0f;
                scaleEnd = animationType == 2 ? trailMask.animation.value().scaleEnd() : 1.0f;

                //if no time slot exists, the start and end time shows the times of the last edited item by design
                //so that editing is more convenient for the user
                if(trailMask.timeSlot) {
                    timeSlot.start = trailMask.timeSlot.value().start;
                    timeSlot.end = trailMask.timeSlot.value().end;
                }
            }

            ImGui::Text("Edit text:");
            ImGui::InputTextMultiline("##edittext", &textToEdit, ImVec2(-FLT_MIN, ImGui::GetTextLineHeight() * 8), ImGuiInputTextFlags_CallbackCharFilter, TextFilters::FilterASCII);

            ImGui::Separator();
            ImGui::Checkbox("Time Slot", &editTimeSlot);

            if(editTimeSlot) {
                editTimeSlotTable(&timeSlot.start, &timeSlot.end);
            }

            ImGui::SliderFloat("Strength", &strength, 0.0f, 5.0f);
            ImGui::SliderFloat("Position Horizontal", &position.x, -1.0f, 1.0f);
            ImGui::SliderFloat("Position Vertical", &position.y, -1.0f, 1.0f);

            ImGui::LinkBegin("##Link Scales", &linkScale, 2.0f);
            ImGui::LinkSliderFloat("Width", &scale.x, 0.1f, 10.0f);
            ImGui::LinkSliderFloat("Height", &scale.y, 0.1f, 10.0f);
            ImGui::LinkEnd();

            ImGui::Text("Animation Type: ");
            ImGui::SameLine();
            ImGui::RadioButton("None", &animationType, 0);
            ImGui::SameLine();
            ImGui::RadioButton("Translation", &animationType, 1);
            ImGui::SameLine();
            ImGui::RadioButton("Scaling", &animationType, 2);

            switch (animationType) {
                case 0: { }  // None
                break;
                case 1: {   // Translation
                    ImGui::InputDouble("Position Horizontal Velocity", &positionVelocity.x, 0.001, 0.01);
                    ImGui::InputDouble("Position Vertical Velocity", &positionVelocity.y, 0.001, 0.01);
                    ImGui::SliderFloat("Start Position Horizontal", &positionStart.x, -1.0f, 1.0f);
                    ImGui::SliderFloat("Start Position Vertical", &positionStart.y, -1.0f, 1.0f);
                    ImGui::SliderFloat("End Position Horizontal", &positionEnd.x, -1.0f, 1.0f);
                    ImGui::SliderFloat("End Position Vertical", &positionEnd.y, -1.0f, 1.0f);
                }
                break;
                case 2: {   // Scaling
                    ImGui::InputDouble("Scale Velocity", &scaleVelocity, 0.01, 0.1);
                    ImGui::SliderFloat("Start Scale", &scaleStart, 0.1f, 10.0f);
                    ImGui::SliderFloat("End Scale", &scaleEnd, 0.1f, 10.0f);
                }
                break;
            }

            ImGui::Checkbox("Invert", &isInverted);

            ImGui::Separator();
            //error messages for user
            if(editTimeSlot && !timeSlotIsValid) {
                ImGui::TextColored(ImVec4(0.9f, 0.0f, 0.0f, 1.0f), "Timeslot duration must be longer than 0s!");
            }
            if(stringsEmpty) {
                ImGui::TextColored(ImVec4(0.9f, 0.0f, 0.0f, 1.0f), "Text cannot be empty!");
            }

            if(ImGui::Button("Ok")) {
                //data validity check
                timeSlotIsValid = !editTimeSlot || (editTimeSlot && timeSlot.isValid());
                stringsEmpty = textToEdit.empty() || name.empty();
                if(timeSlotIsValid && !stringsEmpty) {
                    TrailMaskProperties properties{
                        .text = textToEdit,
                        .strength = strength,
                        .position = position,
                        .scale = scale,
                        .isInverted = isInverted
                    };

                    if(editTimeSlot) {
                        properties.timeSlot = timeSlot;
                    }

                    switch (animationType) {
                        case 0:
                            properties.animation = std::nullopt; // None
                            break;
                        case 1:
                            properties.animation = TrailMaskAnimation{positionVelocity, positionStart, positionEnd};
                            break;
                        case 2:
                            properties.animation = TrailMaskAnimation{scaleVelocity, scaleStart, scaleEnd};
                            break;
                    }

                    presetWindow->notify(UserEvent{EventType::TEXT_PRESET_EDIT, TrailMaskData{name, TrailMaskType::TEXT, properties}});
                    presetWindow->notify(UserEvent{EventType::TEXT_PRESET_APPLY});

                    ImGui::CloseCurrentPopup();
                }
            }

            ImGui::SameLine();

            if(ImGui::Button("Cancel")) {
                ImGui::CloseCurrentPopup();
            }

            ImGui::SameLine();

            //Fontawesome: fa-solid fa-trash-can 
            if(ImGui::Button("\uf2ed Delete")) {
                presetWindow->notify(UserEvent{EventType::TEXT_PRESET_DELETE, TrailMaskData{trailMask.name, trailMask.type}});
                ImGui::CloseCurrentPopup();
            }

            ImGui::EndPopup();
        }
    }

    void textPresetDeleteModal(const char* stringID, TrailMask& trailMask, PresetWindow* presetWindow) {
        if(ImGui::BeginPopupModal(stringID, NULL, ImGuiWindowFlags_AlwaysAutoResize)) {
            
            ImGui::Text("Name:   %s", trailMask.name.c_str());

            ImGui::Separator();

            //Fontawesome: fa-solid fa-trash-can 
            if (ImGui::Button("\uf2ed Delete")) {
                presetWindow->notify(UserEvent{EventType::TEXT_PRESET_DELETE, TrailMaskData{trailMask.name, trailMask.type}});
                ImGui::CloseCurrentPopup();
            }
            ImGui::SameLine();
            if(ImGui::Button("Cancel")) {
                ImGui::CloseCurrentPopup();
            }
            ImGui::EndPopup();
        }
    }

    void editTimeSlotTable(std::chrono::system_clock::time_point* timeSlotStart, std::chrono::system_clock::time_point* timeSlotEnd) {
        ImGui::BeginTable("time_slot_table", 3, ImGuiTableFlags_SizingFixedFit | ImGuiTableFlags_NoHostExtendX);
        
        ImGui::TableNextRow();
        ImGui::TableNextColumn();
        ImGui::AlignTextToFramePadding();
        ImGui::Text("Start: ");

        ImGui::TableNextColumn();
        ImGui::timePicker("start_time", timeSlotStart);
        ImGui::SameLine();
        ImGui::datePicker("start_date", timeSlotStart);

        ImGui::TableNextColumn();
        const std::string formattedStartTime = std::format("{:%A}", *timeSlotStart);
        ImGui::Text("   %s", formattedStartTime.c_str());

        ImGui::TableNextRow(ImGuiTableRowFlags_None, ImGui::GetFrameHeightWithSpacing());
        ImGui::TableNextColumn();
        ImGui::AlignTextToFramePadding();
        ImGui::Text("Duration: ");
        ImGui::TableNextColumn();
        ImGui::TableNextColumn();
        std::chrono::duration duration = *timeSlotEnd - *timeSlotStart;
        const std::string durationStr = std::format("{:%H:%M:%OS}", duration);
        ImGui::Text("   %s", durationStr.c_str());

        ImGui::TableNextRow();
        ImGui::TableNextColumn();
        ImGui::AlignTextToFramePadding();
        ImGui::Text("End: ");

        ImGui::TableNextColumn();
        ImGui::timePicker("end_time", timeSlotEnd);
        ImGui::SameLine();
        ImGui::datePicker("end_date", timeSlotEnd);

        ImGui::TableNextColumn();
        const std::string formattedEndTime = std::format("{:%A} ", *timeSlotEnd);
        ImGui::Text("   %s", formattedEndTime.c_str());
        ImGui::EndTable();
    }
}