#ifndef PRESET_WINDOW_H
#define PRESET_WINDOW_H

#include <cstdint>

#include "base_window.h"
#include "../user_interface.h"

struct TextFilters {
	//filter for ASCII characters (printable characters and newline)
	static int FilterASCII(ImGuiInputTextCallbackData* data) {
		//convert everything to fixed width 32-bit to catch every UNICODE character
		if ((uint32_t)data->EventChar >= (uint32_t)' ' && (uint32_t)data->EventChar <= (uint32_t)'~') {
			return 0;
		} else if((uint32_t)data->EventChar == (uint32_t)'\n') {
			return 0;
		}
		return 1;
	}
};

class PresetWindow : public BaseWindow {

public:
    void render(ApplicationState* appState) override;

private:

    bool linkTrailMaskScales_ = true;

	void behaviourPresetGUI(ApplicationState* appState);
	void colorPresetGUI(ApplicationState* appState);
	void imagePresetGUI(ApplicationState* appState);
	void textPresetGUI(ApplicationState* appState);
};

#endif // PRESET_WINDOW_H