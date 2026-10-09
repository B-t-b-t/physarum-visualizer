#ifndef TRAIL_MASK_ANIMATION_H
#define TRAIL_MASK_ANIMATION_H

#include "../utility/vector_math.h"

/**
 * @brief Enum representing the type of animation for a trail mask.
 * @note None is just a placeholde for initialization, hopefully not used outside class.
 */
enum class AnimationType {
    None,
    Translation,
    Scaling
};

/**
 * @brief Defines an animation of a trail mask. Can either translate or scale, not both simultaneously. 
 *        The animation is simple and moves or scales the trail mask between a start and a destination state.
 *        This class does not represent the position of the actual trail mask!
 */
class TrailMaskAnimation {
public:
    //Constructors, either for translation or scaling
    TrailMaskAnimation(phys::Vec2<double> positionVelocity, phys::Vec2<float> startPos, phys::Vec2<float> endPos);
    TrailMaskAnimation(double scaleVelocity, float scaleStart, float scaleEnd);

    /**
    * @brief Checks if the trail mask has reached its destination in terms of position or(!) scale.
    * 
    * @param trailMaskPosition  position to check
    * @param trailMaskScale     scale to check in the X and Y direction
    * @return True if destination state reached 
    * @return False otherwise
    */
    bool hasReachedDest(phys::Vec2<float> trailMaskPosition, phys::Vec2<float> trailMaskScale) const;

    AnimationType animationType() const { return animationType_; } //get the type of the animation
    bool hasTranslation() const { return animationType_ == AnimationType::Translation; } //legacy, if bool is required
    bool hasScaling() const { return animationType_ == AnimationType::Scaling; } //check if animation is scaling type

    phys::Vec2<double> positionVelocity() const { return positionVelocity_; }   //get the velocity of the translation
    phys::Vec2<float> positionStart() const { return positionStart_; } //get the starting position of the translation
    phys::Vec2<float> positionEnd() const { return positionEnd_; } //get the ending position of the translation

    double scaleVelocity() const { return scaleVelocity_; } //get the velocity of the scaling
    float scaleStart() const { return scaleStart_; } //get the starting scale
    float scaleEnd() const { return scaleEnd_; } //get the ending scale

private:

    AnimationType animationType_{AnimationType::None}; //type of the animation (None, Translation, Scaling)

    phys::Vec2<double> positionVelocity_ = {0.0, 0.0}; //translation speed
    phys::Vec2<float> positionStart_ = {0.0, 0.0};  //starting position of the translation
    phys::Vec2<float> positionEnd_ = {0.0, 0.0};  //ending position of the translation

    double scaleVelocity_ = 0.0;    //scaling speed
    float scaleStart_ = 1.0f;    //starting scale of the scaling
    float scaleEnd_ = 1.0f;  //ending scale of the scaling

};

#endif // TRAIL_MASK_ANIMATION_H