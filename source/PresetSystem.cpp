#include "PresetSystem.h"
#include <fstream>
#include <iostream>
#include <dirent.h>

PresetSystem::PresetSystem(std::string presetFilePath, std::string fileExtension) : presetFilePath_(presetFilePath), fileExtension_(fileExtension) {}

void PresetSystem::createPreset(std::string presetName, UIState &uiState) {

    Preset preset;
    preset.name = presetName;
    preset.useMask = uiState.slimeSettings.useMask;
    preset.collisionDetection = uiState.universalShaderSettings.collisionDetection;
    preset.v = uiState.slimeSettings.v;
    preset.depositionStrength = uiState.slimeSettings.depositionStrength;
    preset.lockAngles = uiState.lockAngles;
    preset.rotationAngle = uiState.slimeSettings.rotationAngle;
    preset.angle = uiState.slimeSettings.angle;
    preset.sensorDistance = uiState.slimeSettings.sensorDistance;
    preset.diffusionWeight = uiState.trailDiffusionSettings.diffusionWeight;
    preset.decay = uiState.trailDiffusionSettings.decay;

    presets.insert({presetName, preset});
}

void PresetSystem::savePreset(std::string fileName) {
    
    std::ofstream file;
    file.open(presetFilePath_ + fileName + fileExtension_);

    Preset preset = presets[fileName];

    file << "Name= " << preset.name << std::endl;
    file << "UseMask= " << preset.useMask << std::endl;
    file << "CollisionDetection= " << preset.collisionDetection << std::endl;
    file << "V= " << preset.v << std::endl;
    file << "DepositionStrength= " << preset.depositionStrength << std::endl;
    file << "LockAngles= " << preset.lockAngles << std::endl;
    file << "RotationAngle= " << preset.rotationAngle << std::endl;
    file << "Angle= " << preset.angle << std::endl;
    file << "SensorDistance= " << preset.sensorDistance << std::endl;
    file << "DiffusionWeight= " << preset.diffusionWeight << std::endl;
    file << "Decay= " << preset.decay << std::endl;
    
    file.close();
}

void PresetSystem::loadPreset(std::string fileName) {
    std::ifstream file;
    file.open(presetFilePath_ + fileName + fileExtension_);
    
    Preset preset;
    std::string line;
    while (std::getline(file, line)) {
        size_t delimPos = line.find("= ");
        if (delimPos == std::string::npos) continue;
        
        std::string key = line.substr(0, delimPos);
        std::string value = line.substr(delimPos + 2);
        
        if (key == "Name")
            preset.name = value;
        else if (key == "UseMask")
            preset.useMask = std::stoi(value);
        else if (key == "CollisionDetection")
            preset.collisionDetection = std::stoi(value);
        else if (key == "V")
            preset.v = std::stof(value);
        else if (key == "DepositionStrength")
            preset.depositionStrength = std::stof(value);
        else if (key == "LockAngles")
            preset.lockAngles = (bool)std::stoi(value);
        else if (key == "RotationAngle")
            preset.rotationAngle = std::stof(value);
        else if (key == "Angle")
            preset.angle = std::stof(value);
        else if (key == "SensorDistance")
            preset.sensorDistance = std::stof(value);
        else if (key == "DiffusionWeight")
            preset.diffusionWeight = std::stof(value);
        else if (key == "Decay")
            preset.decay = std::stof(value);
    }
    
    presets[preset.name] = preset;
    file.close();
}

void PresetSystem::loadPresetNames(UserInterface &ui) {
    DIR *dir;
    struct dirent *ent;
    
    if ((dir = opendir(presetFilePath_.c_str())) != NULL) {
        while ((ent = readdir(dir)) != NULL) {
            std::string fileName = ent->d_name;
            // Check if file has .psf extension
            if (fileName.length() > 4 && 
                fileName.substr(fileName.length() - 4) == fileExtension_) {
                // Remove .psf extension to get preset name
                std::string presetName = fileName.substr(0, fileName.length() - 4);
                loadPreset(presetName);
                ui.addPresetName(presetName);
            }
        }
        closedir(dir);
    }
}

void PresetSystem::loadRandomPreset(UserInterface &ui) {
    unsigned int randomIndex = (unsigned int) (rand() % (int)presets.size());
    ui.setSelectedPreset(randomIndex);
    setUIState(ui.getState(), ui.getSelectedPresetName());
}

void PresetSystem::setUIState(UIState &uiState, std::string presetName) {
    Preset preset = presets[presetName];

    uiState.slimeSettings.useMask = preset.useMask;
    uiState.universalShaderSettings.collisionDetection = preset.collisionDetection;
    uiState.slimeSettings.v = preset.v;
    uiState.slimeSettings.depositionStrength = preset.depositionStrength;
    uiState.lockAngles = preset.lockAngles;
    uiState.slimeSettings.rotationAngle = preset.rotationAngle;
    uiState.slimeSettings.angle = preset.angle;
    uiState.slimeSettings.sensorDistance = preset.sensorDistance;
    uiState.trailDiffusionSettings.diffusionWeight = preset.diffusionWeight;
    uiState.trailDiffusionSettings.decay = preset.decay;
}

void PresetSystem::handleUIRequests(UserInterface &UserInterface) {
    UIState &uiState = UserInterface.getState();

    //Saving Presets to File
    if(uiState.saveToPreset) {
        createPreset(std::string(UserInterface.getLastPresetName()), uiState);
        savePreset(std::string(UserInterface.getLastPresetName()));
        uiState.saveToPreset = false;
    }

    //Loading Presets from File
    if(uiState.loadFromPreset) {
        std::string presetName = std::string(UserInterface.getSelectedPresetName());

        loadPreset(presetName);
        setUIState(uiState, presetName);
        uiState.loadFromPreset = false;
    }
}

void PresetSystem::autoSwitchPresets(UserInterface &UserInterface, Uint64 timeInSeconds) {
    UIState &uiState = UserInterface.getState();

    //Timed Auto Preset Switching
    if(uiState.autoPresetSwitching) {
        uiState.saveToPreset = false;
        uiState.loadFromPreset = false;

        if((timeInSeconds % (Uint64)uiState.presetIntervall == 0) && !timeOut_ && uiState.slimeSettings.velocityBassReaction > uiState.beatVolumeSwitch) {
            loadRandomPreset(UserInterface);
            timeOut_ = true;
        } else if((timeInSeconds % (Uint64)uiState.presetIntervall > 0) && timeOut_){
            timeOut_ = false;
        }
    }
}