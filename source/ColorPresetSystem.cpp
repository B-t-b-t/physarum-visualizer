#include "ColorPresetSystem.h"
#include <fstream>
#include <iostream>
#include <dirent.h>
#include "ui/elements/PresetWindow.h"

ColorPresetSystem::ColorPresetSystem(std::string presetFilePath, std::string fileExtension) : presetFilePath_(presetFilePath), fileExtension_(fileExtension) {}

void ColorPresetSystem::createPreset(std::string presetName, UIState &uiState) {

    ColorPreset preset;
    preset.name = presetName;
    preset.lockSlimeColor = uiState.lockSlimeColor;
    preset.slimeColor0 = uiState.slimeSettings.slimeColor0;
    preset.slimeColor1 = uiState.slimeSettings.slimeColor1;
    preset.slimeColor2 = uiState.slimeSettings.slimeColor2;

    colorPresets.insert({presetName, preset});
}
    
void ColorPresetSystem::savePreset(std::string fileName) {
    
    std::ofstream file;
    file.open(presetFilePath_ + fileName + fileExtension_);

    ColorPreset preset = colorPresets[fileName];

    file << "Name= " << preset.name << std::endl;
    file << "LockSlimeColor= " << preset.lockSlimeColor << std::endl;
    file << "SlimeColor1= " << preset.slimeColor0.x << " " << preset.slimeColor0.y << " " << preset.slimeColor0.z << " " << preset.slimeColor0.w << std::endl;
    file << "SlimeColor2= " << preset.slimeColor1.x << " " << preset.slimeColor1.y << " " << preset.slimeColor1.z << " " << preset.slimeColor1.w << std::endl;
    file << "SlimeColor3= " << preset.slimeColor2.x << " " << preset.slimeColor2.y << " " << preset.slimeColor2.z << " " << preset.slimeColor2.w << std::endl;
    
    
    file.close();
}

void ColorPresetSystem::loadPreset(std::string fileName) {
    std::ifstream file;
    file.open(presetFilePath_ + fileName + fileExtension_);
    
    ColorPreset preset;
    std::string line;
    while (std::getline(file, line)) {
        size_t delimPos = line.find("= ");
        if (delimPos == std::string::npos) continue;
        
        std::string key = line.substr(0, delimPos);
        std::string value = line.substr(delimPos + 2);
        
        if (key == "Name")
            preset.name = value;
        else if (key == "LockSlimeColor")
            preset.lockSlimeColor = (bool)std::stoi(value);
        else if (key == "SlimeColor1")
            sscanf(value.c_str(), "%f %f %f %f", &preset.slimeColor0.x, &preset.slimeColor0.y, &preset.slimeColor0.z, &preset.slimeColor0.w);
        else if (key == "SlimeColor2")
            sscanf(value.c_str(), "%f %f %f %f", &preset.slimeColor1.x, &preset.slimeColor1.y, &preset.slimeColor1.z, &preset.slimeColor1.w);
        else if (key == "SlimeColor3")
            sscanf(value.c_str(), "%f %f %f %f", &preset.slimeColor2.x, &preset.slimeColor2.y, &preset.slimeColor2.z, &preset.slimeColor2.w);
    }

    colorPresets[preset.name] = preset;
    file.close();
}

void ColorPresetSystem::loadPresetNames(UserInterface &ui) {
    DIR *dir;
    struct dirent *ent;
    PresetWindow *window = dynamic_cast<PresetWindow*>(ui.getWindow("PresetWindow"));
    
    if ((dir = opendir(presetFilePath_.c_str())) != NULL) {
        while ((ent = readdir(dir)) != NULL) {
            std::string fileName = ent->d_name;
            // Check if file has .pcsf extension
            if (fileName.length() > 5 && 
                fileName.substr(fileName.length() - 5) == ".pcsf") {
                // Remove .pcsf extension to get preset name
                std::string presetName = fileName.substr(0, fileName.length() - 5);
                loadPreset(presetName);
                window->addColorPresetName(presetName);
            }
        }
        closedir(dir);
    }
}

void ColorPresetSystem::loadRandomPreset(UserInterface &ui) {
    unsigned int randomIndex = (unsigned int) (rand() % (int)colorPresets.size());
    PresetWindow *window = dynamic_cast<PresetWindow*>(ui.getWindow("PresetWindow"));
    
    window->setSelectedColorPreset(randomIndex);
    setUIState(ui.getState(), window->getSelectedColorPresetName());
}

void ColorPresetSystem::setUIState(UIState &uiState, std::string presetName) {
    ColorPreset preset = colorPresets[presetName];

    uiState.lockSlimeColor = preset.lockSlimeColor;
    uiState.slimeSettings.slimeColor0 = preset.slimeColor0;
    uiState.slimeSettings.slimeColor1 = preset.slimeColor1;
    uiState.slimeSettings.slimeColor2 = preset.slimeColor2;
}

void ColorPresetSystem::handleUIRequests(UserInterface &ui) {
    UIState &uiState = ui.getState();
    PresetWindow *window = dynamic_cast<PresetWindow*>(ui.getWindow("PresetWindow"));

    //Saving Color Presets to File
    if(uiState.saveToColorPreset) {
        createPreset(std::string(window->getLastColorPresetName()), uiState);
        savePreset(std::string(window->getLastColorPresetName()));
        uiState.saveToColorPreset = false;
    }

    //Loading Color Presets from File
    if(uiState.loadFromColorPreset) {
        std::string presetName = std::string(window->getSelectedColorPresetName());

        loadPreset(presetName);
        setUIState(uiState, presetName);
        uiState.loadFromColorPreset = false;
    }
}

void ColorPresetSystem::autoSwitchPresets(UserInterface &ui, Uint64 timeInSeconds) {
    UIState &uiState = ui.getState();

    //Timed Auto Preset Switching
    if(uiState.autoPresetSwitching) {
        uiState.saveToPreset = false;
        uiState.loadFromPreset = false;

        if((timeInSeconds % (Uint64)uiState.presetIntervall == 0) && !timeOut_ && uiState.slimeSettings.velocityBassReaction > uiState.beatVolumeSwitch) {
            loadRandomPreset(ui);
            timeOut_ = true;
        } else if((timeInSeconds % (Uint64)uiState.presetIntervall > 0) && timeOut_){
            timeOut_ = false;
        }
    }
}