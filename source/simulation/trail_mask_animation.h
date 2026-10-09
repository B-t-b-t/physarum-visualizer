#ifndef TRAIL_MASK_ANIMATION_H
#define TRAIL_MASK_ANIMATION_H

#include "../utility/vector_math.h"

class TrailMaskAnimation {
public:
    TrailMaskAnimation(phys::Vec2<double> positionVelocity, phys::Vec2<float> startPos, phys::Vec2<float> endPos);
    TrailMaskAnimation(double scaleVelocity, float scaleStart, float scaleEnd);
    bool hasReachedDest(phys::Vec2<float> trailMaskPosition, float trailMaskScaleX, float trailMaskScaleY) const;

    bool hasTranslation() const { return hasTranslation_; }
    bool hasScaling() const { return hasScaling_; }

    phys::Vec2<double> positionVelocity() const { return positionVelocity_; }
    phys::Vec2<float> positionStart() const { return positionStart_; }
    phys::Vec2<float> positionEnd() const { return positionEnd_; }

    double scaleVelocity() const { return scaleVelocity_; }
    float scaleStart() const { return scaleStart_; }
    float scaleEnd() const { return scaleEnd_; }

private:

    bool hasTranslation_{false};
    phys::Vec2<double> positionVelocity_ = {0.0, 0.0}; //option to move the trail mask across the screen
    phys::Vec2<float> positionStart_ = {0.0, 0.0};  //initial position to start from
    phys::Vec2<float> positionEnd_ = {0.0, 0.0};  //final position to move to

    bool hasScaling_{false};
    double scaleVelocity_ = 0.0;    //option to zoom the trail mask
    float scaleStart_ = 1.0f;    //initial scale to start from
    float scaleEnd_ = 1.0f;  //final scale to reach

};

#endif // TRAIL_MASK_ANIMATION_H