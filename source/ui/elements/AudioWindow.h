#pragma once
#include "ImGuiWindow.h"
#include "../UserInterface.h"

class AudioWindow : public ImGuiWindow {

public:
    AudioWindow();
    void render(UIState &state) override;

    void update(std::vector<double>& audioBuffer, std::vector<double>& spectrum, std::vector<double>& spectrumDiff, int bufferSize, bool hasNewSpectrumData);

    void addHardwareDevice(const std::string& deviceName) { availableHardwareDevices_.push_back(deviceName); }
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