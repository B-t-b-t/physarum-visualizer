#ifndef SCENE_H
#define SCENE_H

#include <map>
#include <string>
#include <vector>

#include "../preset_types.h"
#include "../simulation/trail_mask.h"

struct ApplicationState;    // Forward declaration of ApplicationState

class Scene {
public:
    explicit Scene(std::string name);

    void applyRandomPresets(ApplicationState* appState);

    const std::string& getName() const { return name_; }

    void removeAssociatedBehavior(const std::string& behaviorName) { associatedBehaviors_.erase(behaviorName); }
    void removeAssociatedColor(const std::string& colorName) { associatedColors_.erase(colorName); }
    void removeAssociatedImage(const std::string& imageKey) { associatedImages_.erase(imageKey); }
    void removeAssociatedText(const std::string& textKey) { associatedTexts_.erase(textKey); }

    void setAssociatedBehaviors(std::map<std::string, BehaviorPreset>& behaviors, 
                                const std::vector<std::string>& behaviorNames);

    void setAssociatedColors(std::map<std::string, ColorPreset>& colors, 
                             const std::vector<std::string>& colorNames);

    void setAssociatedImages(std::map<std::string, TrailMask>& trailMasks,
                             const std::vector<std::string>& imageKeys);

    void setAssociatedTexts(std::map<std::string, TrailMask>& trailMasks,
                            const std::vector<std::string>& textKeys);

    std::map<std::string, BehaviorPreset*>* getAssociatedBehaviors() { return &associatedBehaviors_; }
    std::map<std::string, ColorPreset*>* getAssociatedColors() { return &associatedColors_; }
    std::map<std::string, TrailMask*>* getAssociatedImages() { return &associatedImages_; }
    std::map<std::string, TrailMask*>* getAssociatedTexts() { return &associatedTexts_; }

private:
    std::string name_;

    std::map<std::string, BehaviorPreset*> associatedBehaviors_;
    std::map<std::string, ColorPreset*> associatedColors_;
    std::map<std::string, TrailMask*> associatedImages_;
    std::map<std::string, TrailMask*> associatedTexts_;
};

#endif // SCENE_H