#include "scene_controller.h"

#include <fstream>
#include <iostream>
#include <string_view>
#include <utility>

#include <toml.hpp>

#include "../application_state.h"
#include "../utility/event.h"

namespace {

    template <typename T_Preset>
    toml::array serializePresetKeys(const std::map<std::string, T_Preset*>& presets) {

        toml::array keys;

        for (const auto& [key, preset] : presets) {
            keys.push_back(key);
        }

        return keys;
    }

} // end anonymous namespace

SceneController::SceneController(
    std::filesystem::path tomlFilePath,
    ApplicationState* appState)
    : tomlFilePath_{std::move(tomlFilePath)},
      appState_{appState} {

    appState_->scenes = &scenes_;
    loadFromToml();
}

SceneController::~SceneController() {
    saveToToml();
}

void SceneController::onNotify(const UserEvent event) {
    switch (event.type) {
        case EventType::SCENE_APPLY: {
            applyScene(std::get<std::string>(event.payload));
        }
        break;

        case EventType::SCENE_CREATE: {
            createScene(std::get<std::string>(event.payload));
        }
        break;
        case EventType::SCENE_DELETE: {
            deleteScene(std::get<std::string>(event.payload));
        }
        break;
        case EventType::SCENE_EDIT: {
            editScene(std::get<SceneEditData>(event.payload));
        }
        break;
        case EventType::BEHAVIOR_PRESET_DELETE: {
            std::string presetToDelete = std::get<std::string>(event.payload);
            for (auto& [sceneName, scene] : scenes_) {
                scene.removeAssociatedBehavior(presetToDelete);
            }
        }
        break;
        case EventType::COLOR_PRESET_DELETE: {
            std::string presetToDelete = std::get<std::string>(event.payload);
            for (auto& [sceneName, scene] : scenes_) {
                scene.removeAssociatedColor(presetToDelete);
            }
        }
        break;
        case EventType::IMAGE_PRESET_DELETE: {
            std::string presetToDelete = std::get<TrailMask>(event.payload).name;
            for (auto& [sceneName, scene] : scenes_) {
                scene.removeAssociatedImage(presetToDelete);
            }
        }
        break;
        case EventType::TEXT_PRESET_DELETE: {
            std::string presetToDelete = std::get<TrailMask>(event.payload).name;
            for (auto& [sceneName, scene] : scenes_) {
                scene.removeAssociatedText(presetToDelete);
            }
        }
        break;
        default:
            break;
    }
}

void SceneController::applyScene(const std::string& sceneName) {
    if (scenes_.contains(sceneName)) {
        activeScene_ = sceneName;
        appState_->usedSceneName = sceneName;
        scenes_.at(sceneName).applyRandomPresets(appState_);
        notify(UserEvent{EventType::BEHAVIOR_PRESET_APPLY, appState_->usedBehaviorPresetName});
        notify(UserEvent{EventType::COLOR_PRESET_APPLY, appState_->usedColorPresetName});
        notify(UserEvent{EventType::IMAGE_PRESET_APPLY, appState_->usedTrailMaskName}); //IMAGE_PREEST_APPLY and TEXT_PRESET_APPLY result in same outcome in TrailMapController
        std::cout << "Applied scene: " << sceneName << std::endl;
    }
}

void SceneController::loadRandomScene() {
    if (!scenes_.empty()) {
        auto it = scenes_.begin();
        std::advance(it, rand() % (int)scenes_.size());
        applyScene(it->first);
    }
}

void SceneController::autoSwitchScenes(uint64_t timeInSeconds) {
    static bool timeOut = false;

    //Timed Auto Preset Switching
    if(appState_->autoSceneSwitching) {
        if((timeInSeconds % (uint64_t)appState_->sceneSwitchingIntervall == 0) && !timeOut && appState_->slimeSettings.velocityBassReaction > appState_->beatVolumeSwitch) {
            loadRandomScene();
            timeOut = true;
        } else if((timeInSeconds % (uint64_t)appState_->sceneSwitchingIntervall > 0) && timeOut){
            timeOut = false;
        }
    }
}

void SceneController::editScene(const SceneEditData& editData) {
    const auto scene = scenes_.find(editData.sceneName);

    if (scene == scenes_.end()) {
        std::cerr << "Can't edit unknown scene '"
                  << editData.sceneName << "'" << std::endl;
        return;
    }

    if (appState_->behaviorPresets == nullptr ||
        appState_->colorPresets == nullptr ||
        appState_->trailMasks == nullptr) 
    {
        std::cerr << "Cannot edit scene '" << editData.sceneName
                  << "' because one or more preset collections are unavailable"
                  << std::endl;
        return;
    }

    scene->second.setAssociatedBehaviors(
        *appState_->behaviorPresets,
        editData.behaviorPresetNames);

    scene->second.setAssociatedColors(
        *appState_->colorPresets,
        editData.colorPresetNames);

    scene->second.setAssociatedImages(
        *appState_->trailMasks,
        editData.imagePresetKeys);

    scene->second.setAssociatedTexts(
        *appState_->trailMasks,
        editData.textPresetKeys);
}

void SceneController::loadFromToml() {
    toml::value sceneData{};

    try {
        sceneData = toml::parse(tomlFilePath_);
    } catch (const toml::exception& err) {
        std::cerr << "Failed to parse " << tomlFilePath_.filename()
                  << ": " << err.what() << std::endl;
        return;
    }

    for (const auto& [sceneName, sceneEntry] : sceneData.as_table()) {
        scenes_.try_emplace(sceneName, sceneName);

        editScene(SceneEditData{
            sceneName,
            toml::find_or(
                sceneEntry,
                "BehaviorPresets",
                std::vector<std::string>{}),
            toml::find_or(
                sceneEntry,
                "ColorPresets",
                std::vector<std::string>{}),
            toml::find_or(
                sceneEntry,
                "ImagePresets",
                std::vector<std::string>{}),
            toml::find_or(
                sceneEntry,
                "TextPresets",
                std::vector<std::string>{})
        });
    }
}

bool SceneController::saveToToml() {
    toml::value sceneData{toml::table{}};

    for (auto& [sceneName, scene] : scenes_) {
        sceneData[sceneName] = toml::table{
            {"Name", scene.getName()},
            {
                "BehaviorPresets",
                serializePresetKeys(*scene.getAssociatedBehaviors())
            },
            {
                "ColorPresets",
                serializePresetKeys(*scene.getAssociatedColors())
            },
            {
                "ImagePresets",
                serializePresetKeys(*scene.getAssociatedImages())
            },
            {
                "TextPresets",
                serializePresetKeys(*scene.getAssociatedTexts())
            }
        };
    }

    std::ofstream outputFile{tomlFilePath_, std::ios::trunc};

    if (!outputFile.is_open()) {
        std::cerr << "Failed to open " << tomlFilePath_
                  << " for writing" << std::endl;
        return false;
    }

    outputFile << toml::format(sceneData);
    outputFile.flush();

    if (!outputFile.good()) {
        std::cerr << "Failed to write to " << tomlFilePath_ << std::endl;
        return false;
    }

    return true;
}