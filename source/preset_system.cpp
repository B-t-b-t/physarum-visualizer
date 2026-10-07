#include "preset_system.h"

#include <fstream>              // for basic_ostream, basic_ofstream, operat...
#include <iostream>             // for cerr
#include <iterator>             // for next
#include <memory>               // for make_unique
#include <stdlib.h>             // for rand
#include "toml.hpp"             // for basic_value, format_error, make_error...
#include <type_traits>          // for is_same_v
#include <utility>              // for get
#include <variant>              // for get

#include "application_state.h"  // for ApplicationState
#include "preset_types.h"       // for BehaviorPreset, ColorPreset
#include "uniforms.h"           // for SlimeSettings
#include "utility/event.h"      // for EventType, UserEvent

//only a few Preset Types necessary
template class PresetSystem<BehaviorPreset>;
template class PresetSystem<ColorPreset>;

template<typename T>
PresetSystem<T>::PresetSystem(std::filesystem::path presetFilePath, ApplicationState* appState)
 : presetFilePath_{presetFilePath}, appState_{appState}
{
    //register all preset names with UI and presets into memory
    loadPresetsFromFile();
    
    //differentiate between BehaviorPreset and ColorPreset pointers
    if constexpr (std::is_same_v<T, BehaviorPreset>) {
        appState_->behaviorPresets = &presets;
        appState_->usedBehaviorPresetName = presets.empty() ? "" : presets.begin()->first;
    } else if constexpr (std::is_same_v<T, ColorPreset>) {
        appState_->colorPresets = &presets;
        appState_->usedColorPresetName = presets.empty() ? "" : presets.begin()->first;
    }
}

template<typename T>
PresetSystem<T>::~PresetSystem() {
    savePresetsToFile();
}

template<typename T>
void PresetSystem<T>::createPreset(std::string presetName) {
    presets.insert({presetName, T{presetName, appState_}});
}

template<typename T>
void PresetSystem<T>::savePresetsToFile() {

    toml::value presetData{toml::table{}};
    
    for(auto& preset : presets) {
        presetData[preset.first] = preset.second.toTomlTable();
    }

    std::ofstream outputFile{presetFilePath_, std::ios::trunc};
    if(!outputFile.is_open()) {
        std::cerr << "Failed to open " << presetFilePath_.filename() << " for writing" << std::endl;
    }

    outputFile << toml::format(presetData);
    outputFile.flush();

    if(!outputFile.good()) {
        std::cerr << "Failed to write " << presetFilePath_.filename() << std::endl;
    }
}

template<typename T>
void PresetSystem<T>::loadPresetsFromFile() {
    toml::value presetData{};

    //parse
    try {
        presetData = toml::parse(presetFilePath_);
    } catch(const toml::exception& err) {
        std::cerr << "Failed to parse " << presetFilePath_.filename() << ": " << err.what() << std::endl;
    }

    //fill map
    for(auto& [presetName, presetEntry] : presetData.as_table()) {    
        presets[presetName.c_str()] = T{presetName.c_str(), presetEntry};
    }
}

template<typename T>
void PresetSystem<T>::onNotify(const UserEvent event) {

    switch (event.type) {
        case EventType::BEHAVIOR_PRESET_CREATE:
        case EventType::COLOR_PRESET_CREATE:
            createPreset(std::get<std::string>(event.payload));
            break;
        case EventType::BEHAVIOR_PRESET_APPLY: 
        case EventType::COLOR_PRESET_APPLY: {
            presets[std::get<std::string>(event.payload)].toAppState(appState_);
            break;
        }
        case EventType::BEHAVIOR_PRESET_DELETE:
        case EventType::COLOR_PRESET_DELETE:
            presets.erase(std::get<std::string>(event.payload));
            break;
        default:
            break;
    }
}