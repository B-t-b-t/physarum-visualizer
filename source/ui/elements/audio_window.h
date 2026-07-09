#ifndef AUDIO_WINDOW_H
#define AUDIO_WINDOW_H

#include "imgui_window.h"
#include "../user_interface.h"

class AudioWindow : public ImGuiWindow {

public:
    AudioWindow();
    void render(ApplicationState* appState) override;

    void update(std::vector<double>& audioBuffer, std::vector<double>& spectrum, std::vector<double>& spectrumDiff, int bufferSize, bool hasNewSpectrumData);

    void addHardwareDeviceNames(const std::vector<std::string>& deviceNames);
	const char * getSelectedHardwareDevice() { return availableHardwareDevices_[selectedHardwareDevice_].c_str(); }

private:
	std::vector<std::string> availableHardwareDevices_;
	unsigned int selectedHardwareDevice_ = 0;

    std::vector<double>* audioBuffer_ = nullptr;
    std::vector<double>* spectrum_ = nullptr;
    std::vector<double>* spectrumDiff_ = nullptr;
    int bufferSize_ = 0;
    bool hasNewSpectrumData_ = false;

	std::vector<double> heatmapData_;
	std::vector<double> heatMapChange_;
	int heatMapIndex_ = 0;
};

#endif // AUDIO_WINDOW_H