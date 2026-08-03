#ifndef AUDIO_WINDOW_H
#define AUDIO_WINDOW_H

#include "base_window.h"
#include "../user_interface.h"
#include "../../application_state.h"

class AudioWindow : public BaseWindow {

public:
    AudioWindow();
    void render(ApplicationState* appState) override;

    void addHardwareDeviceNames(const std::vector<std::string>& deviceNames);
	const char * getSelectedHardwareDevice() { return availableHardwareDevices_[selectedHardwareDevice_].c_str(); }

private:
	std::vector<std::string> availableHardwareDevices_;
	unsigned int selectedHardwareDevice_ = 0;

	std::vector<double> heatmapData_;
	std::vector<double> heatMapChange_;
	int heatMapIndex_ = 0;
};

#endif // AUDIO_WINDOW_H