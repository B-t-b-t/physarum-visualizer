#include "scenes_window.h"

#include <map>
#include <string>

#include "imgui.h"

#include "../../simulation/scene_controller.h"
#include "../../simulation/trail_mask.h"
#include "../../utility/event.h"
#include "../../../external/imgui_stdlib.h"
#include "../widgets/dual_listbox.h"

void sceneAddModal(
    const char* stringID,
    ApplicationState* appState,
    ScenesWindow* window);

void sceneEditModal(
    const char* stringID,
    ApplicationState* appState,
    ScenesWindow* window);

void sceneDeleteModal(
    const char* stringID,
    ApplicationState* appState,
    ScenesWindow* window);

void ScenesWindow::render(ApplicationState* appState) {
    ImGui::Begin("Scenes Window", &visible);

    ImGui::SeparatorText("Scenes");

    ImGui::SetNextWindowSizeConstraints(
        ImVec2(
            ImGui::GetContentRegionAvail().x * 0.5f,
            ImGui::GetTextLineHeightWithSpacing() * 1),
        ImVec2(
            ImGui::GetContentRegionAvail().x * 0.5f,
            ImGui::GetTextLineHeightWithSpacing() * 8));

    ImGui::BeginChild(
        "SceneSelectionChild",
        ImVec2(0, 0),
        true,
        ImGuiWindowFlags_MenuBar);

    std::map<std::string, Scene>* scenes = appState->scenes;

    if (ImGui::BeginMenuBar()) {
        if (ImGui::MenuItem("\uf0fe Add")) {
            ImGui::OpenPopup("New Scene");
        }
        sceneAddModal("New Scene", appState, this);

        if (scenes != nullptr && !scenes->empty()) {
            if (ImGui::MenuItem("\uf044 Edit")) {
                ImGui::OpenPopup("Edit Scene");
            }
            sceneEditModal("Edit Scene", appState, this);

            if (ImGui::MenuItem("\uf2ed Delete")) {
                ImGui::OpenPopup("Delete Scene");
            }
            sceneDeleteModal("Delete Scene", appState, this);
        }

        ImGui::EndMenuBar();
    }

    if (ImGui::BeginTable(
            "##Scene Selection",
            1,
            ImGuiTableFlags_SizingFixedFit)) {

        if (scenes != nullptr && !scenes->empty()) {
            for (const auto& [sceneName, scene] : *scenes) {
                ImGui::TableNextRow();
                ImGui::TableNextColumn();

                const bool isSelected =
                    sceneName == appState->usedSceneName;

                if (ImGui::Selectable(
                        sceneName.c_str(),
                        isSelected,
                        ImGuiSelectableFlags_SpanAllColumns)) {
                    appState->usedSceneName = sceneName;
                    notify(UserEvent{EventType::SCENE_APPLY, sceneName});
                }
            }
        }

        ImGui::EndTable();
    }

    ImGui::EndChild();

    ImGui::SeparatorText("Settings");

    ImGui::SliderInt(
        "Switch Intervall",
        &appState->presetIntervall,
        2,
        60,
        "%d s");

    ImGui::End();
}

void sceneAddModal(
    const char* stringID,
    ApplicationState* appState,
    ScenesWindow* window) {

    if (ImGui::BeginPopupModal(
            stringID,
            nullptr,
            ImGuiWindowFlags_AlwaysAutoResize)) {

        static std::string newSceneName;
        static bool sceneAlreadyExists = false;

        ImGui::InputTextWithHint(
            "##New Scene Name",
            "Name",
            &newSceneName,
            ImGuiInputTextFlags_CharsNoBlank);

        ImGui::Separator();

        if (ImGui::Button("Save") && !newSceneName.empty()) {
            sceneAlreadyExists = appState->scenes->contains(newSceneName);

            if (!sceneAlreadyExists) {
                window->notify(UserEvent{
                    EventType::SCENE_CREATE,
                    newSceneName
                });

                newSceneName.clear();
                ImGui::CloseCurrentPopup();
            }
        }

        ImGui::SameLine();

        if (ImGui::Button("Cancel")) {
            newSceneName.clear();
            sceneAlreadyExists = false;
            ImGui::CloseCurrentPopup();
        }

        if (sceneAlreadyExists && ImGui::IsItemHovered()) {
            ImGui::TextUnformatted("A scene with this name already exists.");
        }

        ImGui::EndPopup();
    }
}

void sceneDeleteModal(
    const char* stringID,
    ApplicationState* appState,
    ScenesWindow* window) {

    if (ImGui::BeginPopupModal(
            stringID,
            nullptr,
            ImGuiWindowFlags_AlwaysAutoResize)) {

        ImGui::Text("Name: %s", appState->usedSceneName.c_str());

        ImGui::Separator();

        if (ImGui::Button("\uf2ed Delete")) {
            window->notify(UserEvent{
                EventType::SCENE_DELETE,
                appState->usedSceneName
            });

            ImGui::CloseCurrentPopup();
        }

        ImGui::SameLine();

        if (ImGui::Button("Cancel")) {
            ImGui::CloseCurrentPopup();
        }

        ImGui::EndPopup();
    }
}

void sceneEditModal(
    const char* stringID,
    ApplicationState* appState,
    ScenesWindow* window) {

    if (!ImGui::BeginPopupModal(
            stringID,
            nullptr,
            ImGuiWindowFlags_AlwaysAutoResize)) {
        return;
    }

    static DualListBox behaviorList;
    static DualListBox colorList;
    static DualListBox imageList;
    static DualListBox textList;

    const bool hasPresetData =
        appState->scenes != nullptr &&
        appState->behaviorPresets != nullptr &&
        appState->colorPresets != nullptr &&
        appState->trailMasks != nullptr;

    const auto scene =
        hasPresetData
            ? appState->scenes->find(appState->usedSceneName)
            : std::map<std::string, Scene>::iterator{};

    const bool canEdit =
        hasPresetData &&
        scene != appState->scenes->end();

    if (canEdit && ImGui::IsWindowAppearing()) {
        behaviorList.reset(
            *appState->behaviorPresets,
            *scene->second.getAssociatedBehaviors(),
            [](const BehaviorPreset&) {
                return true;
            },
            [](const std::string& key, const BehaviorPreset&) {
                return key;
            });

        colorList.reset(
            *appState->colorPresets,
            *scene->second.getAssociatedColors(),
            [](const ColorPreset&) {
                return true;
            },
            [](const std::string& key, const ColorPreset&) {
                return key;
            });

        imageList.reset(
            *appState->trailMasks,
            *scene->second.getAssociatedImages(),
            [](const TrailMask& trailMask) {
                return trailMask.type == TrailMaskType::IMAGE;
            },
            [](const std::string&, const TrailMask& trailMask) {
                return trailMask.name;
            });

        textList.reset(
            *appState->trailMasks,
            *scene->second.getAssociatedTexts(),
            [](const TrailMask& trailMask) {
                return trailMask.type == TrailMaskType::TEXT;
            },
            [](const std::string&, const TrailMask& trailMask) {
                return trailMask.name;
            });
    }

    if (canEdit) {
        ImGui::Text("Name: %s", scene->first.c_str());

        if (ImGui::BeginTabBar("Preset Types")) {
            if (ImGui::BeginTabItem("\uf02a Behaviour")) {
                behaviorList.show("BehaviorList");
                ImGui::EndTabItem();
            }

            if (ImGui::BeginTabItem("\uf53f Color")) {
                colorList.show("ColorList");
                ImGui::EndTabItem();
            }

            if (ImGui::BeginTabItem("\uf302 Image")) {
                imageList.show("ImageList");
                ImGui::EndTabItem();
            }

            if (ImGui::BeginTabItem("\uf031 Text")) {
                textList.show("TextList");
                ImGui::EndTabItem();
            }

            ImGui::EndTabBar();
        }
    } else {
        ImGui::TextUnformatted(
            "The selected scene or preset collections are unavailable.");
    }

    ImGui::Separator();

    if (!canEdit) {
        ImGui::BeginDisabled();
    }

    if (ImGui::Button("Save") && canEdit) {
        window->notify(UserEvent{
            EventType::SCENE_EDIT,
            SceneEditData{
                scene->first,
                behaviorList.getActivePresetKeys(),
                colorList.getActivePresetKeys(),
                imageList.getActivePresetKeys(),
                textList.getActivePresetKeys()
            }
        });

        ImGui::CloseCurrentPopup();
    }

    if (!canEdit) {
        ImGui::EndDisabled();
    }

    ImGui::SameLine();

    if (ImGui::Button("Cancel")) {
        ImGui::CloseCurrentPopup();
    }

    ImGui::EndPopup();
}