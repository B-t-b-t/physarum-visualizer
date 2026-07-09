#include "color_preset_system.h"

#include <fstream>
#include <iostream>

#include "ui/elements/preset_window.h"
#include "utility/fileHandling.h"

ColorPresetSystem::ColorPresetSystem(std::string presetFilePath, std::string fileExtension, UserInterface* ui)
 : presetFilePath_(presetFilePath), fileExtension_(fileExtension), ui_(ui), appState_{ui_->getState()}
{
    //register all preset names with UI and presets into memory
    loadPresetNames(ui_);
}

void ColorPresetSystem::createPreset(std::string presetName, ApplicationState* appState) {

    ColorPreset preset;
    preset.name = presetName;
    preset.lockSlimeColor = appState->lockSlimeColor;
    preset.slimeColor0 = appState->slimeSettings.slimeColor0;
    preset.slimeColor1 = appState->slimeSettings.slimeColor1;
    preset.slimeColor2 = appState->slimeSettings.slimeColor2;

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

void ColorPresetSystem::loadPresetNames(UserInterface* ui) {
    PresetWindow *window = dynamic_cast<PresetWindow*>(ui->getWindow("PresetWindow"));
    std::vector<std::string> presetNames;
    loadFileNames(presetFilePath_, fileExtension_, presetNames);

    for (std::string &presetName : presetNames) {
        window->addColorPresetName(presetName);
        loadPreset(presetName);
    }
}

void ColorPresetSystem::loadRandomPreset(UserInterface* ui) {
    if(!colorPresets.empty()) {
        unsigned int randomIndex = (unsigned int) (rand() % (int)colorPresets.size());
        PresetWindow *window = dynamic_cast<PresetWindow*>(ui->getWindow("PresetWindow"));
        window->setSelectedColorPreset(randomIndex);
        setUIState(ui_->getState(), window->getSelectedColorPresetName());
    } else {
        std::cerr << "WARN: No color presets available to auto switch" << std::endl;
    }
}

void ColorPresetSystem::setUIState(ApplicationState* appState, std::string presetName) {
    ColorPreset preset = colorPresets[presetName];

    appState->lockSlimeColor = preset.lockSlimeColor;
    appState->slimeSettings.slimeColor0 = preset.slimeColor0;
    appState->slimeSettings.slimeColor1 = preset.slimeColor1;
    appState->slimeSettings.slimeColor2 = preset.slimeColor2;
}

void ColorPresetSystem::autoSwitchPresets(UserInterface* ui, Uint64 timeInSeconds) {
    ApplicationState* appState = ui->getState();

    //Timed Auto Preset Switching
    if(appState->autoPresetSwitching) {
        if((timeInSeconds % (Uint64)appState->colorPresetIntervall == 0) && !timeOut_ && appState->slimeSettings.velocityBassReaction > appState->beatVolumeSwitch) {
            loadRandomPreset(ui);
            timeOut_ = true;
        } else if((timeInSeconds % (Uint64)appState->colorPresetIntervall > 0) && timeOut_){
            timeOut_ = false;
        }
    }
}

void ColorPresetSystem::onNotify(const Event event) {
    PresetWindow *window = dynamic_cast<PresetWindow*>(observable_);

    switch (event) {
        case Event::SAVE_COLOR_PRESET:
            createPreset(std::string(window->getLastColorPresetName()), appState_);
            savePreset(std::string(window->getLastColorPresetName()));
            break;
        case Event::LOAD_COLOR_PRESET: {
            std::string presetName = std::string(window->getSelectedColorPresetName());
            loadPreset(presetName);
            setUIState(appState_, presetName);
            break;
        }
        default:
            break;
    }
}