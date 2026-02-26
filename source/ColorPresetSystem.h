#ifndef COLORPRESETSYSTEM_H
#define COLORPRESETSYSTEM_H

#include <string>
#include <unordered_map>
#include "../imgui/imgui.h"
#include "./ui/UserInterface.h"

struct ColorPreset {
    std::string name;
    
	bool lockSlimeColor;
	ImVec4 slimeColor0;
	ImVec4 slimeColor1;
	ImVec4 slimeColor2;
};

class ColorPresetSystem {
public:

	ColorPresetSystem(std::string presetFilePath, std::string fileExtension);

    void createPreset(std::string presetName, UIState &uiState);
    void savePreset(std::string fileName);
    void loadPreset(std::string fileName);
	void loadRandomPreset(UserInterface &ui);

	void loadPresetNames(UserInterface &ui);
	void setUIState(UIState &uiState, std::string presetName);

	void handleUIRequests(UserInterface &ui);
	void autoSwitchPresets(UserInterface &ui, Uint64 timeInSeconds);

private:

    std::unordered_map<std::string, ColorPreset> colorPresets;

	std::string presetFilePath_;
	std::string fileExtension_;

	bool timeOut_ = false;
};

#endif // COLORPRESETSYSTEM_H