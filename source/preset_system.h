#ifndef PRESET_SYSTEM_H
#define PRESET_SYSTEM_H

#include <string>
#include <unordered_map>

#include "imgui.h"

#include "./ui/user_interface.h"
#include "./utility/observer.h"

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

class PresetSystem : public Observer{
public:

	PresetSystem() = default;
	PresetSystem(std::string presetFilePath, std::string fileExtension, UserInterface* ui);

    void createPreset(std::string presetName, UIState* uiState);
    void savePreset(std::string fileName);
    void loadPreset(std::string fileName);
	void loadRandomPreset(UserInterface* ui);

	void loadPresetNames(UserInterface* ui);
	void setUIState(UIState* uiState, std::string presetName);

	void autoSwitchPresets(UserInterface* ui, Uint64 timeInSeconds);

	void onNotify(const Event event) override;

private:

    std::unordered_map<std::string, Preset> presets;

	std::string presetFilePath_;
	std::string fileExtension_;
	UserInterface* ui_;

	UIState* uiState_ = UIState::getInstance();

	bool timeOut_ = false;
};

#endif // PRESET_SYSTEM_H