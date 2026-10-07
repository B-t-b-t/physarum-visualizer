#include "scene.h"

#include <iostream>

#include "../application_state.h"
#include "../utility/time_handling.h"

namespace {

template <typename T_Preset, typename Predicate>
std::map<std::string, T_Preset*> resolveAssociatedPresets(const std::string& sceneName,
                                                          std::map<std::string, T_Preset>& presets,
                                                          const std::vector<std::string>& presetKeys,
                                                          std::string_view presetType,
                                                          Predicate isAcceptedPreset) 
{
    std::map<std::string, T_Preset*> associatedPresets;

    for (const std::string& key : presetKeys) {
        const auto preset = presets.find(key);

        if (preset == presets.end()) {
            std::cerr << "Scene '" << sceneName
                    << "' references unknown " << presetType
                    << " preset '" << key << "'" << std::endl;
            continue;
        }

        if (!isAcceptedPreset(preset->second)) {
            std::cerr << "Scene '" << sceneName
                    << "' references an incompatible " << presetType
                    << " preset '" << key << "'" << std::endl;
            continue;
        }

        associatedPresets.try_emplace(preset->first, &preset->second);
    }

    return associatedPresets;
}

}   // end anonymous namespace


Scene::Scene(std::string name)
    : name_{std::move(name)} {
}

void Scene::setAssociatedBehaviors(std::map<std::string, BehaviorPreset>& behaviors,
                                   const std::vector<std::string>& behaviorNames) 
{
    associatedBehaviors_ = resolveAssociatedPresets(
        name_,
        behaviors,
        behaviorNames,
        "behavior",
        [](const BehaviorPreset&) {
            return true;
        });
}

void Scene::setAssociatedColors(std::map<std::string, ColorPreset>& colors,
                                const std::vector<std::string>& colorNames) 
{
    associatedColors_ = resolveAssociatedPresets(
        name_,
        colors,
        colorNames,
        "color",
        [](const ColorPreset&) {
            return true;
        });
}

void Scene::setAssociatedImages(std::map<std::string, TrailMask>& trailMasks,
                                const std::vector<std::string>& imageKeys) 
{
    associatedImages_ = resolveAssociatedPresets(
        name_,
        trailMasks,
        imageKeys,
        "image",
        [](const TrailMask& trailMask) {
            return trailMask.type == TrailMaskType::IMAGE;
        });
}

void Scene::setAssociatedTexts(std::map<std::string, TrailMask>& trailMasks,
    const std::vector<std::string>& textKeys) 
{
    associatedTexts_ = resolveAssociatedPresets(
        name_,
        trailMasks,
        textKeys,
        "text",
        [](const TrailMask& trailMask) {
            return trailMask.type == TrailMaskType::TEXT;
        });
}

void Scene::applyRandomPresets(ApplicationState* appState) {
    long int randomIndex = 0;
    size_t length = 0;
    std::string selectedKey = "";

    //randomly select a behavior preset
    length = associatedBehaviors_.size();
    if(associatedBehaviors_.size() > 0) {
        randomIndex = static_cast<long int>((size_t)rand() % length);
        selectedKey = std::next(associatedBehaviors_.begin(), randomIndex)->first;
        appState->usedBehaviorPresetName = selectedKey;
    }

    //randomly select a color preset
    length = associatedColors_.size();
    if(length > 0) {
        randomIndex = static_cast<long int>((size_t)rand() % length);
        selectedKey = std::next(associatedColors_.begin(), randomIndex)->first;
        appState->usedColorPresetName = selectedKey;
    }

    //randomly select a trail mask
    //give each image and text an equal chance to get selected
    length = associatedImages_.size() + associatedTexts_.size();
    if(length > 0) {
        bool isValid = true;
        size_t attempts = 0;    //to prevent infinite loop if all timeslots are outside the current time

        do {
            attempts++;
            randomIndex = static_cast<long int>((size_t)rand() % length);
            //determine if an image or a text was selected
            if(randomIndex < static_cast<long int>(associatedImages_.size())) {
                selectedKey = std::next(associatedImages_.begin(), randomIndex)->first;
                //check timetable
                if(associatedImages_.at(selectedKey)->timeSlot) {
                    isValid = associatedImages_.at(selectedKey)->timeSlot->isNow();
                }

            } else {
                selectedKey = std::next(associatedTexts_.begin(), randomIndex - static_cast<long int>(associatedImages_.size()))->first;
                //check timetable
                if(associatedTexts_.at(selectedKey)->timeSlot) {
                    isValid = associatedTexts_.at(selectedKey)->timeSlot->isNow();
                }
            }
        } while(!isValid && attempts < length); // because of random() length is not a guaranteed limit for the count of necessary attempts, but still a good approximation

        if(isValid) { appState->usedTrailMaskName = selectedKey; }
    }
}