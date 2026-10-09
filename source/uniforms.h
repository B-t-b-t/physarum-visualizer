#ifndef UNIFORMS_H
#define UNIFORMS_H

#include "utility/vector_math.h"  // for phys::Vec2, phys::Vec4

// custom vector types for GPU alignment (std140)
struct alignas(8) Std140Vec2 { float x; float y; };
static_assert(sizeof(Std140Vec2) == 8); //just to make sure...
static_assert(alignof(Std140Vec2) == 8);

struct alignas(16) Std140Vec4 { float x; float y; float z; float w; };
static_assert(sizeof(Std140Vec4) == 16);
static_assert(alignof(Std140Vec4) == 16);

// conversion from and to phys::Vec types
[[nodiscard]] constexpr Std140Vec2 toStd140(const phys::Vec2<float>& value) {
    return {value.x, value.y};
}

[[nodiscard]] constexpr Std140Vec4 toStd140(const phys::Vec4<float>& value) {
    return {value.x, value.y, value.z, value.w};
}

[[nodiscard]] constexpr phys::Vec2<float> fromStd140(const Std140Vec2& value) {
    return {value.x, value.y};
}

[[nodiscard]] constexpr phys::Vec4<float> fromStd140(const Std140Vec4& value) {
    return {value.x, value.y, value.z, value.w};
}

struct alignas(16) UniversalShaderSettings {
    int textureWidth = 1600;
    int textureHeight = 896;
    int windowWidth = 1600;
    int windowHeight = 896;

    int renderParticles = false;
    int renderCollisions = false;
    int collisionDetection = false;
    int timeTicks = 0;

    float trailMaskInfluence = 1.0f;
    Std140Vec2 trailMaskScale = toStd140(phys::Vec2<float>{1.0f, 1.0f});
    float _padding0 = 0.0f;

    Std140Vec2 trailMaskPosition = toStd140(phys::Vec2<float>{0.0f, 0.0f});
    int trailMaskIsInverted = false;
    float _padding1;

    Std140Vec4 mouseInputs = toStd140(phys::Vec4<float>{0.0f, 0.0f, 0.0f, 0.0f}); // x, y, leftClick, rightClick
};
static_assert(sizeof(UniversalShaderSettings) % 16 == 0, "UniversalShaderSettings size must be multiple of 16 for std140");
    

// Aligned for std140 layout
struct alignas(16) SlimeSettings {
    float v = 1.5f;
    float depositionStrength = 1.0f;
    int rotationAngle = 20;
    int angle = 20;

    int rotationAngleBias = 0;
    int sensingAngleBias = 0;
    int sensorDistance = 9;
    unsigned int densityLimit = 10;

    int useMask = false;
    float velocityBassReaction = 0.0f;
    int reactToAudio = false;
    int angleBassReaction = 0;

    Std140Vec4 slimeColor0 = toStd140(phys::Vec4<float>{0.0f, 1.0f, 1.0f, 1.0f});
    Std140Vec4 slimeColor1 = toStd140(phys::Vec4<float>{0.0f, 1.0f, 1.0f, 1.0f});
    Std140Vec4 slimeColor2 = toStd140(phys::Vec4<float>{0.0f, 1.0f, 1.0f, 1.0f});
    Std140Vec4 particleColor0 = toStd140(phys::Vec4<float>{1.0f, 0.0f, 0.0f, 1.0f});
    Std140Vec4 particleColor1 = toStd140(phys::Vec4<float>{0.0f, 1.0f, 0.0f, 1.0f});
    Std140Vec4 particleColor2 = toStd140(phys::Vec4<float>{0.0f, 0.0f, 1.0f, 1.0f});
    Std140Vec4 collisionColor = toStd140(phys::Vec4<float>{1.0f, 1.0f, 1.0f, 1.0f});

    float velocityBiasX = 0.0f;
    float velocityBiasY = 0.0f;
    float _padding2 = 0.0f;
    float _padding3 = 0.0f;
};
static_assert(sizeof(SlimeSettings) % 16 == 0, "SlimeSettings size must be multiple of 16 for std140");

// Aligned for std140 layout
struct alignas(16) TrailDiffusionSettings {
    float diffusionWeight = 1.0f;
    float decay = 0.5f;
    int useTrailMask = true;
    int _padding1; // Padding for alignment
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
    float bloomThreshold = 0.05f;
    float bloomKnee = 0.01f; // soft knee width for bloom threshold
    float bloomBassReaction = 0.0f;

    float brightnessMultiplier = 0.0f;
    int   positionDebugOverlay = false;
    int lineThickness = 1;
    int _padding3; // Padding for alignment
};
static_assert(sizeof(FragmentShaderSettings) % 16 == 0, "FragmentShaderSettings size must be multiple of 16 for std140");

// Aligned for std140 layout
struct alignas(16) ParameterSettings {
    float p1 = 0.0f;
    float p2 = 0.0f;
    float p3 = 1.0f;
    float p4 = 0.0f;

    float p5 = 0.0f;
    float p6 = 1.0f;
    float p7 = 0.0f;
    float p8 = 0.0f;
    
    float p9 = 1.0f;
    float p10 = 0.0f;
    float p11 = 0.0f;
    float p12 = 1.0f;

    int enableParameters = false;
    int _padding1; // Padding for alignment
    int _padding2; // Padding for alignment
    int _padding3; // Padding for alignment
};
static_assert(sizeof(ParameterSettings) % 16 == 0, "ParameterSettings size must be multiple of 16 for std140");

#endif // UNIFORMS_H