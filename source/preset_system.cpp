#include "preset_system.h"

#include <fstream>          // for basic_ostream, basic_ofstream, operator<<
#include <iostream>         // for cerr
#include <iterator>         // for next
#include <memory>           // for make_unique
#include <stdlib.h>         // for rand
#include <utility>          // for pair, get
#include <variant>          // for get

#include "toml.hpp"         // for make_error_info, region::as_string, eithe...

#include "utility/event.h"  // for UserEvent, EventType

//only a few Preset Types necessary
template class PresetSystem<BehaviorPreset>;
template class PresetSystem<ColorPreset>;

template<typename T>
PresetSystem<T>::PresetSystem(std::string presetFilePath, ApplicationState* appState)
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
void PresetSystem<T>::loadRandomPreset() {

   if(!presets.empty()) {
        unsigned int randomIndex = (unsigned int) (rand() % (int)presets.size());
        presets[std::next(presets.begin(), randomIndex)->first].toAppState(appState_);
        appState_->usedBehaviorPresetName = std::next(presets.begin(), randomIndex)->first;
    } else {
        std::cerr << "WARN: No presets available to auto switch" << std::endl;
    }
}

template<typename T>
void PresetSystem<T>::autoSwitchPresets(Uint64 timeInSeconds) {

    //Timed Auto Preset Switching
    if(appState_->autoPresetSwitching) {
        if((timeInSeconds % (Uint64)appState_->presetIntervall == 0) && !timeOut_ && appState_->slimeSettings.velocityBassReaction > appState_->beatVolumeSwitch) {
            loadRandomPreset();
            timeOut_ = true;
        } else if((timeInSeconds % (Uint64)appState_->presetIntervall > 0) && timeOut_){
            timeOut_ = false;
        }
    }
}

template<typename T>
void PresetSystem<T>::onNotify(const UserEvent event) {

    switch (event.type) {
        case EventType::SAVE_PRESET:
        case EventType::SAVE_COLOR_PRESET:
            createPreset(std::get<std::string>(event.data_1));
            break;
        case EventType::LOAD_PRESET: 
        case EventType::LOAD_COLOR_PRESET: {
            presets[std::get<std::string>(event.data_1)].toAppState(appState_);
            break;
        }
        default:
            break;
    }
}