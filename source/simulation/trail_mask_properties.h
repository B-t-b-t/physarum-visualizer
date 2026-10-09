#ifndef TRAIL_MASK_PROPERTIES_H
#define TRAIL_MASK_PROPERTIES_H

#include <optional>
#include <string>

#include "trail_mask_animation.h"  // for TrailMaskAnimation
#include "../utility/time_handling.h"  // for TimeSlot
#include "../utility/vector_math.h"    // for phys::Vec2

/**
 * @brief Properties of a trail mask.
 * 
 * Used only for the constructor of TrailMask, so when defining only some properties, 
 * the user isn't constrained by the order of the constructor parameters.
 * 
 * @note Defined in it's own header file for unit testing without including all dependencies of Trail Mask.
 */
struct TrailMaskProperties {
        std::string text{""};
        std::optional<TimeSlot> timeSlot = std::nullopt;

        float strength{1.0f};
        phys::Vec2<float> position{0.0f, 0.0f};
        phys::Vec2<float> scale{1.0f, 1.0f};    //external user defined scale

        std::optional<TrailMaskAnimation> animation = std::nullopt;

        bool isInverted{false};
};

enum class TrailMaskType {
    IMAGE,
    TEXT,
    EMPTY       //makes it possible for the user to have "no" trail mask selected (TODO: find better solution)
};

#endif // TRAIL_MASK_PROPERTIES_H