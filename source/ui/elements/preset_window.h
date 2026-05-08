#ifndef PRESET_WINDOW_H
#define PRESET_WINDOW_H

#include "imgui_window.h"
#include "../user_interface.h"

class PresetWindow : public ImGuiWindow {

public:
    void render(UIState &state) override;

    void addPresetName(const std::string& presetName) { presetNames_.push_back(presetName); }
	const char * getLastPresetName() { return presetNames_.back().c_str(); }
	const char * getSelectedPresetName() { return presetNames_[selectedPresetName_].c_str(); }
	void setSelectedPreset(unsigned int index) { selectedPresetName_ = index;  }

	void addColorPresetName(const std::string& colorPresetName) { colorPresetNames_.push_back(colorPresetName); }
	const char * getLastColorPresetName() { return colorPresetNames_.back().c_str(); }
	const char * getSelectedColorPresetName() { return colorPresetNames_[selectedColorPresetName_].c_str(); }
	void setSelectedColorPreset(unsigned int index) { selectedColorPresetName_ = index;  }

	void addPictureName(const std::string& pictureName) { pictureNames_.push_back(pictureName); }
	const char * getLastPictureName() { return pictureNames_.back().c_str(); }
	const char * getSelectedPictureName() { return pictureNames_[selectedPictureName_].c_str(); }
	void setSelectedPicture(unsigned int index) { selectedPictureName_ = index; }

private:
	std::vector<std::string> presetNames_;
	unsigned int selectedPresetName_ = 0;
	std::vector<std::string> colorPresetNames_;
	unsigned int selectedColorPresetName_ = 0;

	std::vector<std::string> pictureNames_;
	unsigned int selectedPictureName_ = 0;
};

#endif // PRESET_WINDOW_H