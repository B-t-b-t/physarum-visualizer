#ifndef PRESET_WINDOW_H
#define PRESET_WINDOW_H

#include "base_window.h"
#include "../user_interface.h"

struct TextFilters {
	//filter for ASCII characters (printable characters and newline)
	static int FilterASCII(ImGuiInputTextCallbackData* data) {
		if ((unsigned char)data->EventChar >= ' ' && (unsigned char)data->EventChar <= '~') {
			return 0;
		} else if((unsigned char)data->EventChar == '\n') {
			return 0;
		}
		return 1;
	}
};

class PresetWindow : public BaseWindow {

public:
    void render(ApplicationState* appState) override;

    void addPresetName(const std::string& presetName) { presetNames_.push_back(presetName); }
	const char * getLastPresetName() { return presetNames_.back().c_str(); }
	const char * getSelectedPresetName() { return presetNames_[selectedPresetName_].c_str(); }
	void setSelectedPreset(unsigned int index) { selectedPresetName_ = index;  }

	void addColorPresetName(const std::string& colorPresetName) { colorPresetNames_.push_back(colorPresetName); }
	const char * getLastColorPresetName() { return colorPresetNames_.back().c_str(); }
	const char * getSelectedColorPresetName() { return colorPresetNames_[selectedColorPresetName_].c_str(); }
	void setSelectedColorPreset(unsigned int index) { selectedColorPresetName_ = index;  }

private:
	std::vector<std::string> presetNames_;
	unsigned int selectedPresetName_ = 0;
	std::vector<std::string> colorPresetNames_;
	unsigned int selectedColorPresetName_ = 0;

	bool isEditingTextPreset_{false};
	std::string textToEdit_{""};

	void behaviourPresetGUI(ApplicationState* appState);
	void colorPresetGUI(ApplicationState* appState);
	void imagePresetGUI(ApplicationState* appState);
	void textPresetGUI(ApplicationState* appState);
};

#endif // PRESET_WINDOW_H