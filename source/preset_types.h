#ifndef PRESET_TYPES_H
#define PRESET_TYPES_H

#include <string>
#include <vector>

#include "toml.hpp"

#include "application_state.h"

struct BehaviorPreset {
    std::string name;
    
	bool useMask;
	bool collisionDetection;
	float v;
	float depositionStrength;
	bool lockAngles;
	int rotationAngle;
	int angle;
	int lockAngleBiases;
	int rotationAngleBias;
	int angleBias;
	int sensorDistance;
	float diffusionWeight;
	float decay;

	BehaviorPreset() = default;

	BehaviorPreset(std::string presetName, ApplicationState* appState) {
		name = presetName;
		useMask = appState->slimeSettings.useMask;
		collisionDetection = appState->universalShaderSettings.collisionDetection;
		v = appState->slimeSettings.v;
		depositionStrength = appState->slimeSettings.depositionStrength;
		lockAngles = appState->lockAngles;
		rotationAngle = appState->slimeSettings.rotationAngle;
		angle = appState->slimeSettings.angle;
		lockAngleBiases = appState->lockAngleBiases;
		rotationAngleBias = appState->slimeSettings.rotationAngleBias;
		angleBias = appState->slimeSettings.sensingAngleBias;
		sensorDistance = appState->slimeSettings.sensorDistance;
		diffusionWeight = appState->trailDiffusionSettings.diffusionWeight;
		decay = appState->trailDiffusionSettings.decay;
	}

	BehaviorPreset(std::string presetName, toml::value presetEntry) {
        name = presetName;
        useMask = toml::find_or(presetEntry, "UseMask", false);
        collisionDetection = toml::find_or(presetEntry, "CollisionDetection", false);
        v = toml::find_or(presetEntry, "V", 0.0f);
        depositionStrength = toml::find_or(presetEntry, "DepositionStrength", 0.0f);
        lockAngles = toml::find_or(presetEntry, "LockAngles", false);
        rotationAngle = toml::find_or(presetEntry, "RotationAngle", 0);
        angle = toml::find_or(presetEntry, "Angle", 0);
        lockAngleBiases = toml::find_or(presetEntry, "LockAngleBiases", 0);
        rotationAngleBias = toml::find_or(presetEntry, "RotationAngleBias", 0);
        angleBias = toml::find_or(presetEntry, "AngleBias", 0);
        sensorDistance = toml::find_or(presetEntry, "SensorDistance", 0);
        diffusionWeight = toml::find_or(presetEntry, "DiffusionWeight", 0.0f);
        decay = toml::find_or(presetEntry, "Decay", 0.0f);
	}

	void toAppState(ApplicationState* appState) {
		appState->slimeSettings.useMask = useMask;
		appState->universalShaderSettings.collisionDetection = collisionDetection;
		appState->slimeSettings.v = v;
		appState->slimeSettings.depositionStrength = depositionStrength;
		appState->lockAngles = lockAngles;
		appState->slimeSettings.rotationAngle = rotationAngle;
		appState->slimeSettings.angle = angle;
		appState->lockAngleBiases = lockAngleBiases;
		appState->slimeSettings.rotationAngleBias = rotationAngleBias;
		appState->slimeSettings.sensingAngleBias = angleBias;
		appState->slimeSettings.sensorDistance = sensorDistance;
		appState->trailDiffusionSettings.diffusionWeight = diffusionWeight;
		appState->trailDiffusionSettings.decay = decay;
	}

	toml::table toTomlTable() {
		return toml::table{
				{"Name", name},
				{"UseMask", useMask},
				{"CollisionDetection", collisionDetection},
				{"DepositionStrength", depositionStrength},
				{"LockAngles", lockAngles},
				{"RotationAngle", rotationAngle},
				{"Angle", angle},
				{"LockAngleBiases", lockAngleBiases},
				{"RotationAngleBias", rotationAngleBias},
				{"V", v},
				{"AngleBias", angleBias},
				{"SensorDistance", sensorDistance},
				{"Decay", decay},
				{"DiffusionWeight", diffusionWeight}
		};
	}
};

struct ColorPreset {
    std::string name;
    
	bool lockSlimeColor;
	ImVec4 slimeColor0;
	ImVec4 slimeColor1;
	ImVec4 slimeColor2;

    ColorPreset() = default;

    ColorPreset(std::string presetName, ApplicationState* appState) {
        name = presetName;
        lockSlimeColor = appState->lockSlimeColor;
        slimeColor0 = appState->slimeSettings.slimeColor0;
        slimeColor1 = appState->slimeSettings.slimeColor1;
        slimeColor2 = appState->slimeSettings.slimeColor2;
    }

    ColorPreset(std::string presetName, const toml::value presetEntry) {
        name = presetName;
        lockSlimeColor = toml::find_or(presetEntry, "LockSlimeColor", false);

        std::vector<float> tempColor0 = toml::find<std::vector<float>>(presetEntry, "SlimeColor1");
        std::vector<float> tempColor1 = toml::find<std::vector<float>>(presetEntry, "SlimeColor2");
        std::vector<float> tempColor2 = toml::find<std::vector<float>>(presetEntry, "SlimeColor3");

        slimeColor0 = ImVec4{tempColor0[0], tempColor0[1], tempColor0[2], tempColor0[3]};
        slimeColor1 = ImVec4{tempColor1[0], tempColor1[1], tempColor1[2], tempColor1[3]};
        slimeColor2 = ImVec4{tempColor2[0], tempColor2[1], tempColor2[2], tempColor2[3]};
    }

    void toAppState(ApplicationState* appState) {
        appState->lockSlimeColor = lockSlimeColor;
        appState->slimeSettings.slimeColor0 = slimeColor0;
        appState->slimeSettings.slimeColor1 = slimeColor1;
        appState->slimeSettings.slimeColor2 = slimeColor2;
    }

    toml::table toTomlTable() {
        return toml::table{
                {"Name", name},
                {"LockSlimeColor", lockSlimeColor},
                {"SlimeColor1", toml::array{slimeColor0.x, slimeColor0.y, slimeColor0.z, slimeColor0.w}},
                {"SlimeColor2", toml::array{slimeColor1.x, slimeColor1.y, slimeColor1.z, slimeColor1.w}},
                {"SlimeColor3", toml::array{slimeColor2.x, slimeColor2.y, slimeColor2.z, slimeColor2.w}}
        };
    }
};


#endif // PRESET_TYPES_H