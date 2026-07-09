#include "preset_system.h"

#include <fstream>
#include <iostream>

#include "ui/elements/preset_window.h"
#include "utility/fileHandling.h"

PresetSystem::PresetSystem(std::string presetFilePath, std::string fileExtension, UserInterface* ui)
 : presetFilePath_(presetFilePath), fileExtension_(fileExtension), ui_(ui), appState_(ui->getState())
{
    //register all preset names with UI and presets into memory
    loadPresetNames(ui);
}

void PresetSystem::createPreset(std::string presetName, ApplicationState* uiState) {

    Preset preset;
    preset.name = presetName;
    preset.useMask = uiState->slimeSettings.useMask;
    preset.collisionDetection = uiState->universalShaderSettings.collisionDetection;
    preset.v = uiState->slimeSettings.v;
    preset.depositionStrength = uiState->slimeSettings.depositionStrength;
    preset.lockAngles = uiState->lockAngles;
    preset.rotationAngle = uiState->slimeSettings.rotationAngle;
    preset.angle = uiState->slimeSettings.angle;
    preset.sensorDistance = uiState->slimeSettings.sensorDistance;
    preset.diffusionWeight = uiState->trailDiffusionSettings.diffusionWeight;
    preset.decay = uiState->trailDiffusionSettings.decay;

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

void PresetSystem::loadPresetNames(UserInterface* ui) {

    PresetWindow *window = dynamic_cast<PresetWindow*>(ui->getWindow("PresetWindow"));
    std::vector<std::string> presetNames;
    loadFileNames(presetFilePath_, fileExtension_, presetNames);

    for (std::string &presetName : presetNames) {
        window->addPresetName(presetName);
        loadPreset(presetName);
    }
}

void PresetSystem::loadRandomPreset(UserInterface* ui) {

   if(!presets.empty()) {
        PresetWindow *window = dynamic_cast<PresetWindow*>(ui->getWindow("PresetWindow"));
        unsigned int randomIndex = (unsigned int) (rand() % (int)presets.size());
        window->setSelectedPreset(randomIndex);
        setUIState(ui->getState(), window->getSelectedPresetName());
    } else {
        std::cerr << "WARN: No presets available to auto switch" << std::endl;
    }
}

void PresetSystem::setUIState(ApplicationState* uiState, std::string presetName) {
    Preset preset = presets[presetName];

    uiState->slimeSettings.useMask = preset.useMask;
    uiState->universalShaderSettings.collisionDetection = preset.collisionDetection;
    uiState->slimeSettings.v = preset.v;
    uiState->slimeSettings.depositionStrength = preset.depositionStrength;
    uiState->lockAngles = preset.lockAngles;
    uiState->slimeSettings.rotationAngle = preset.rotationAngle;
    uiState->slimeSettings.angle = preset.angle;
    uiState->slimeSettings.sensorDistance = preset.sensorDistance;
    uiState->trailDiffusionSettings.diffusionWeight = preset.diffusionWeight;
    uiState->trailDiffusionSettings.decay = preset.decay;
}

void PresetSystem::autoSwitchPresets(UserInterface* ui, Uint64 timeInSeconds) {
    ApplicationState* uiState = ui->getState();

    //Timed Auto Preset Switching
    if(uiState->autoPresetSwitching) {
        if((timeInSeconds % (Uint64)uiState->presetIntervall == 0) && !timeOut_ && uiState->slimeSettings.velocityBassReaction > uiState->beatVolumeSwitch) {
            loadRandomPreset(ui);
            timeOut_ = true;
        } else if((timeInSeconds % (Uint64)uiState->presetIntervall > 0) && timeOut_){
            timeOut_ = false;
        }
    }
}

void PresetSystem::onNotify(const Event event) {
    PresetWindow *window = dynamic_cast<PresetWindow*>(observable_);

    switch (event) {
        case Event::SAVE_PRESET:
            createPreset(std::string(window->getLastPresetName()), appState_);
            savePreset(std::string(window->getLastPresetName()));
            break;
        case Event::LOAD_PRESET: {
            std::string presetName = std::string(window->getSelectedPresetName());
            loadPreset(presetName);
            setUIState(appState_, presetName);
            break;
        }
        default:
            break;
    }
}