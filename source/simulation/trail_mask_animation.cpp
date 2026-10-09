#include "trail_mask_animation.h"

TrailMaskAnimation::TrailMaskAnimation(phys::Vec2<double> positionVelocity, phys::Vec2<float> startPos, phys::Vec2<float> endPos) {
    hasTranslation_ = true;
    this->positionVelocity_ = positionVelocity;
    positionStart_ = startPos;
    positionEnd_ = endPos;
}

TrailMaskAnimation::TrailMaskAnimation(double scaleVelocity, float scaleStart, float scaleEnd) {
    hasScaling_ = true;
    scaleVelocity_ = scaleVelocity;
    scaleStart_ = scaleStart;
    scaleEnd_ = scaleEnd;
}

bool TrailMaskAnimation::hasReachedDest(phys::Vec2<float> trailMaskPosition, float trailMaskScaleX, float trailMaskScaleY) const {
    bool reachedDestination = false;
    if(hasTranslation_) {
        reachedDestination = phys::VecMath<float>::abs(trailMaskPosition - positionEnd_) < phys::Vec2<float>{0.01, 0.01};
        return reachedDestination;
    } else if(hasScaling_) {
        reachedDestination = std::abs(trailMaskScaleX - scaleEnd_) < 0.01f ||
                                std::abs(trailMaskScaleY - scaleEnd_) < 0.01f;
        return reachedDestination;
    } else {
        return true;    //if there's no movement, it has "reached" its destination by default
    }
}