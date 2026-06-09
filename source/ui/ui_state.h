#ifndef UI_STATE_H
#define UI_STATE_H

#include <string>

#include "../uniforms.h"


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
	ParameterSettings parameterSettings;
	
	bool newCanvas = false;
	bool exitProgram = false;
	bool lockSlimeColor = false;
	bool lockParticleColor = false;
	bool lockAngles = true;
	bool fullscreen = false;
	bool autoPresetSwitching = false;
	std::string currentAudioHardware = "";
	int newTextureWidth = 1600;
	int newTextureHeight = 896;
	int numParticles = 300000;
	int newNumParticles = 300000;
	float slimeRatio = 0.15f;
	int presetIntervall = 30;	//in seconds
	int colorPresetIntervall = 30;	//in seconds
	int trailMaskIntervall = 30;	//in seconds

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

	bool bloomAudioReaction = true;
	float bloomBassReactionIntensity = 1.0f;

	bool normalizeBeat = false;

	static UIState* getInstance();

private:
	static UIState* singleton_;

	UIState() = default;
	~UIState() = default;
	UIState(const UIState&) = delete;
	UIState& operator=(const UIState&) = delete;
};

#endif // UI_STATE_H