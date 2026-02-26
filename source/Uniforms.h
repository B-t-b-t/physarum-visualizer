#pragma once
#include "../imgui/imgui.h"

struct alignas(16) UniversalShaderSettings {
    int textureWidth = 1600;
    int textureHeight = 896;
    int windowWidth = 1600;
    int windowHeight = 896;

    int renderParticles = false;
    int renderCollisions = false;
    int collisionDetection = false;
    int timeTicks = 0;

    ImVec4 mouseInputs = ImVec4(0.0f, 0.0f, 0.0f, 0.0f); // x, y, leftClick, rightClick
};
static_assert(sizeof(UniversalShaderSettings) % 16 == 0, "UniversalShaderSettings size must be multiple of 16 for std140");
    

// Aligned for std140 layout
struct alignas(16) SlimeSettings {
    float v = 1.5f;
    float depositionStrength = 1.0f;
    int rotationAngle = 45;
    int angle = 45;

    int sensorDistance = 9;
    float collisionFraction = 1.0f;
    int useMask = true;
    float velocityBassReaction = 0.0f;

    int reactToAudio = false;
    int angleBassReaction = 0;
    int _padding1; // Padding for alignment
    int _padding2; // Padding for alignment

    ImVec4 slimeColor0 = ImVec4(1.0f, 0.0f, 0.0f, 1.0f);
    ImVec4 slimeColor1 = ImVec4(0.0f, 1.0f, 0.0f, 1.0f);
    ImVec4 slimeColor2 = ImVec4(0.0f, 0.0f, 1.0f, 1.0f);
    ImVec4 particleColor0 = ImVec4(1.0f, 0.0f, 0.0f, 1.0f);
    ImVec4 particleColor1 = ImVec4(0.0f, 1.0f, 0.0f, 1.0f);
    ImVec4 particleColor2 = ImVec4(0.0f, 0.0f, 1.0f, 1.0f);
    ImVec4 collisionColor = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
};
static_assert(sizeof(SlimeSettings) % 16 == 0, "SlimeSettings size must be multiple of 16 for std140");

// Aligned for std140 layout
struct alignas(16) TrailDiffusionSettings {
    float diffusionWeight = 1.0f;
    float decay = 0.5f;
    int _padding1; // Padding for alignment
    int _padding2; // Padding for alignment
};
static_assert(sizeof(TrailDiffusionSettings) % 16 == 0, "TrailDiffusionSettings size must be multiple of 16 for std140");


struct alignas(16) FragmentShaderSettings { 
    float exposure = 1.0f;
    int renderColorTraces = false;
    int toneMappingMode = 2;    //0 = reinhard, 1 = exposure, 2 = ACES
    int vignetteEffect = false;

    float vignetteXDimension = 1.0f;
    float vignetteYDimension = 1.0f;
    float vignetteInnerRadius = 1.0f;
    float vignetteSharpness = 10.0f;

    int vignetteSelector = 0;
    int debugTextureMaskSelector = 0;
    int bloomEnabled = true;
    int bloomBlendMode = 1; //0 = additive, 1 = screen, 2 = soft additive

    float bloomIntensity = 1.0f;
    float bloomThreshold = 0.2f;
    float bloomKnee = 0.0f; // soft knee width for bloom threshold
    float bloomBassReaction = 0.0f;

    float brightnessMultiplier = 0.0f;
    int _padding1; // Padding for alignment
    int _padding2; // Padding for alignment
    int _padding3; // Padding for alignment
};
static_assert(sizeof(FragmentShaderSettings) % 16 == 0, "FragmentShaderSettings size must be multiple of 16 for std140");   
    