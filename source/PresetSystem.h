#ifndef PRESETSYSTEM_H
#define PRESETSYSTEM_H

#include <string>
#include <unordered_map>
#include "imgui.h"
#include "./ui/UserInterface.h"

struct Preset {
    std::string name;
    
	bool useMask;
	bool collisionDetection;
	float v;
	float depositionStrength;
	bool lockAngles;
	int rotationAngle;
	int angle;
	int sensorDistance;
	float diffusionWeight;
	float decay;
};

class PresetSystem {
public:

	PresetSystem(std::string presetFilePath, std::string fileExtension);

    void createPreset(std::string presetName, UIState &uiState);
    void savePreset(std::string fileName);
    void loadPreset(std::string fileName);
	void loadRandomPreset(UserInterface &ui);

	void loadPresetNames(UserInterface &ui);
	void setUIState(UIState &uiState, std::string presetName);

	void handleUIRequests(UserInterface &ui);
	void autoSwitchPresets(UserInterface &ui, Uint64 timeInSeconds);

private:

    std::unordered_map<std::string, Preset> presets;

	std::string presetFilePath_;
	std::string fileExtension_;

	bool timeOut_ = false;
};

#endif // PRESETSYSTEM_H