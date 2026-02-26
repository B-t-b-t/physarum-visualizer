#pragma once
#include "imgui.h"
#include "imgui_impl_sdl3.h"
#include "imgui_impl_opengl3.h"
#include "implot.h"
#include <string>
#include <vector>
#include <math.h>
#include <iostream>
#include "../audio/AudioRecording.h"
#include "../Uniforms.h"

enum class TextureMask {
        TRAIL = 0,
        TRAIL_NON_DIFFUSED = 1,
        NEW_PARTICLES = 2,
        OLD_PARTICLES = 3,
        COLLISIONS = 4,
        BLOOM = 5,
		UPSAMPLE_1 = 6,
		DOWNSAMPLE_2 = 7,
		UPSAMPLE_2 = 8,
		DOWNSAMPLE_3 = 9,
		UPSAMPLE_3 = 10,
		DOWNSAMPLE_4 = 11,
		UPSAMPLE_4 = 12,
		DOWNSAMPLE_5 = 13,
		UPSAMPLE_5 = 14,
		THRESHOLD = 15
};

enum class BloomBlendMode {
		ADDITIVE = 0,
		SCREEN = 1,
		SOFT_ADDITIVE = 2

};

enum class ToneMappingMode {
		REINHARD = 0,
		EXPOSURE = 1
};

class UIState {
public:
	UniversalShaderSettings universalShaderSettings;
	SlimeSettings slimeSettings;
	TrailDiffusionSettings trailDiffusionSettings;
	FragmentShaderSettings fragmentShaderSettings;

	static UIState& getInstance() {
		static UIState instance;
        return instance;
	}
	
	bool newCanvas = false;
	bool exitProgram = false;
	bool lockSlimeColor = false;
	bool lockParticleColor = false;
	bool lockAngles = true;
	bool fullscreen = false;
	bool saveToPreset = false;
	bool loadFromPreset = false;
	bool saveToColorPreset = false;
	bool loadFromColorPreset = false;
	bool autoPresetSwitching = false;
	bool selectAudioHardware = false;
	int newTextureWidth = 1600;
	int newTextureHeight = 896;
	int numParticles = 300000*1;
	int newNumParticles = 300000*1;
	int presetIntervall = 30;	//in seconds

	TextureMask selectedTextureMask = TextureMask::TRAIL;

	ImVec4 clearColor = ImVec4(0.0f, 0.0f, 0.0f, 0.0f);

	float beatDivide = 20.0f;
	bool subBassDetected = false;
	bool bassDetected = false;
	bool lowMidRangeDetected = false;
	bool midRangeDetected = false;
	bool upperMidRangeDetected = false;
	bool presenceDetected = false;
	bool brillianceDetected = false;

	double subBassValue = 0.0;
	double bassValue = 0.0;
	double lowMidRangeValue = 0.0;
	double midRangeValue = 0.0;
	double upperMidRangeValue = 0.0;
	double presenceValue = 0.0;
	double brillianceValue = 0.0;
	
	int beatVolumeSwitch = 5;

	bool normalizeBeat = false;

private:
    UIState() = default;
    ~UIState() = default;
    UIState(const UIState&) = delete;
    UIState& operator=(const UIState&) = delete;
};

class UserInterface {

public:

	UserInterface(SDL_Window* window, SDL_GLContext glContext);
	~UserInterface();

	void display(std::vector<double>& audioBuffer, std::vector<double>& spectrum, std::vector<double>& spectrumDiff, int bufferSize, bool hasNewSpectrumData);

	void addPresetName(const std::string& presetName) { presetNames_.push_back(presetName); }
	const char * getLastPresetName() { return presetNames_.back().c_str(); }
	const char * getSelectedPresetName() { return presetNames_[selectedPresetName_].c_str(); }
	void setSelectedPreset(unsigned int index) { selectedPresetName_ = index;  }

	void addColorPresetName(const std::string& colorPresetName) { colorPresetNames_.push_back(colorPresetName); }
	const char * getLastColorPresetName() { return colorPresetNames_.back().c_str(); }
	const char * getSelectedColorPresetName() { return colorPresetNames_[selectedColorPresetName_].c_str(); }
	void setSelectedColorPreset(unsigned int index) { selectedColorPresetName_ = index;  }

	void addHardwareDevice(const std::string& deviceName) { availableHardwareDevices_.push_back(deviceName); }
	const char * getSelectedHardwareDevice() { return availableHardwareDevices_[selectedHardwareDevice_].c_str(); }
	UIState& getState() { return state_; }

private:

	UIState& state_;
	bool showNew_ = false;
	bool showSlimeGUI_ = true;
	bool showVisualSettingsGUI_ = true;
	bool showPresetGUI_ = false;
	bool showAudioGUI_ = false;
	bool showDebugGUI_ = true;
	bool showImGUIDemo_ = false;
	bool showImPlotDemo_ = false;

	std::vector<std::string> presetNames_;
	unsigned int selectedPresetName_ = 0;
	std::vector<std::string> colorPresetNames_;
	unsigned int selectedColorPresetName_ = 0;

	std::vector<std::string> availableHardwareDevices_;
	unsigned int selectedHardwareDevice_ = 0;

	std::vector<double> heatmapData_;
	std::vector<double> heatMapChange_;
	int heatMapIndex_ = 0;

	void newModal();
	void mainMenuBarGUI();
	void slimeGUI();
	void visualSettingsGUI();
	void presetGUI();
	void audioGUI(std::vector<double>& audioBuffer, std::vector<double>& spectrum, std::vector<double>& spectrumDiff, int bufferSize, bool hasNewSpectrumData);
	void debugGUI(ImGuiIO& guiIO);
};

ImPlotPoint MyDataGetter(int idx, void* data) ;

int TimeFormatter(double value, char* buff, int size, void* data);

int FrequencyFormatter(double value, char* buff, int size, void* data);