#include "trail_mask_animation.h"

#include <iostream>

//Constructor for translation
TrailMaskAnimation::TrailMaskAnimation(phys::Vec2<float> startPos, phys::Vec2<float> endPos, float animationTime) {
    animationType_ = AnimationType::Translation;
    animationTime_ = animationTime;
    positionStart_ = startPos;
    positionEnd_ = endPos;
    phys::Vec2<float> directionVector_ = endPos - startPos;
    positionVelocity_ = phys::Vec2<double>{directionVector_.x / (animationTime_ * 60), directionVector_.y / (animationTime_ * 60)};     // velocity per frame assuming 60 FPS
}

//Constructor for scaling
TrailMaskAnimation::TrailMaskAnimation(float scaleStart, float scaleEnd, float animationTime) {
    animationType_ = AnimationType::Scaling;
    animationTime_ = animationTime;
    scaleStart_ = scaleStart;
    scaleEnd_ = scaleEnd;
    scaleVelocity_ = (scaleEnd_ - scaleStart_) / static_cast<double>(animationTime_ * 60);     // velocity per frame assuming 60 FPS
}

bool TrailMaskAnimation::hasReachedDest(const phys::Vec2<float> trailMaskPosition, const phys::Vec2<float> trailMaskScale) const {
    bool reachedDestination = false;
    
    switch (animationType_) {
        case AnimationType::Translation: {
            float newDistance = phys::VecMath<float>::distance(trailMaskPosition, positionEnd_);
            return newDistance < 2 * phys::VecMath<double>::length(positionVelocity_);
        }
        break;
        case AnimationType::Scaling: {
            if(scaleVelocity_ > 0) {
                reachedDestination = trailMaskScale.x >= scaleEnd_ && trailMaskScale.y >= scaleEnd_;
            } else if(scaleVelocity_ < 0) {
                reachedDestination = trailMaskScale.x <= scaleEnd_ && trailMaskScale.y <= scaleEnd_;
            } else {
                reachedDestination = true; // no scaling, end animation
            }
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