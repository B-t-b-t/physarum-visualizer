#ifndef COLOR_PRESET_SYSTEM_H
#define COLOR_PRESET_SYSTEM_H

#include <string>
#include <unordered_map>

#include "imgui.h"

#include "./ui/ui_state.h"
#include "./ui/user_interface.h"
#include "./utility/observer.h"

struct ColorPreset {
    std::string name;
    
	bool lockSlimeColor;
	ImVec4 slimeColor0;
	ImVec4 slimeColor1;
	ImVec4 slimeColor2;
};

class ColorPresetSystem : public Observer {
public:

	ColorPresetSystem() = default;
	ColorPresetSystem(std::string presetFilePath, std::string fileExtension, UserInterface *ui);

    void createPreset(std::string presetName, UIState* uiState);
    void savePreset(std::string fileName);
    void loadPreset(std::string fileName);
	void loadRandomPreset(UserInterface* ui);

	void loadPresetNames(UserInterface* ui);
	void setUIState(UIState* uiState, std::string presetName);

	void autoSwitchPresets(UserInterface* ui, Uint64 timeInSeconds);

	void onNotify(const Event event) override;

private:

    std::unordered_map<std::string, ColorPreset> colorPresets;

	std::string presetFilePath_;
	std::string fileExtension_;
	UserInterface* ui_;

	bool timeOut_ = false;

	UIState* uiState_;
};

#endif // COLOR_PRESET_SYSTEM_H