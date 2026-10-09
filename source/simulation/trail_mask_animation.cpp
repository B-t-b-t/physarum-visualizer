#include "trail_mask_animation.h"

#include <iostream>

//Constructor for translation
TrailMaskAnimation::TrailMaskAnimation(phys::Vec2<double> positionVelocity, phys::Vec2<float> startPos, phys::Vec2<float> endPos) {
    animationType_ = AnimationType::Translation;
    this->positionVelocity_ = positionVelocity;
    positionStart_ = startPos;
    positionEnd_ = endPos;
}

//Constructor for scaling
TrailMaskAnimation::TrailMaskAnimation(double scaleVelocity, float scaleStart, float scaleEnd) {
    animationType_ = AnimationType::Scaling;
    scaleVelocity_ = scaleVelocity;
    scaleStart_ = scaleStart;
    scaleEnd_ = scaleEnd;
}

bool TrailMaskAnimation::hasReachedDest(phys::Vec2<float> trailMaskPosition, phys::Vec2<float> trailMaskScale) const {
    bool reachedDestination = false;
    constexpr float EPSILON = 0.01f;
    
    switch (animationType_) {
        case AnimationType::Translation: {
            reachedDestination = phys::VecMath<float>::abs(trailMaskPosition - positionEnd_) < phys::Vec2<float>{EPSILON, EPSILON};
            return reachedDestination;
        }
        break;
        case AnimationType::Scaling: {
            reachedDestination = std::abs(trailMaskScale.x - scaleEnd_) < EPSILON
                              || std::abs(trailMaskScale.y - scaleEnd_) < EPSILON;
            return reachedDestination;
        }
        break;
        case AnimationType::None: {
            std::cerr << "TrailMaskAnimation not correctly initialized." << std::endl;
            return true;  //to avoid problems in the caller, consider it as having reached its destination
        }
        break;
        default: {
            std::cerr << "Unknown animation type." << std::endl;
            return true;  //to avoid problems in the caller, consider it as having reached its destination
        }
    }
}