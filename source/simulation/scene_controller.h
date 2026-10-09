#ifndef SCENE_CONTROLLER_H
#define SCENE_CONTROLLER_H

#include <filesystem>
#include <map>
#include <string>

#include "scene.h"
#include "../application_state.h"
#include "../utility/observable.h"
#include "../utility/observer.h"

struct SceneEditData;   // Forward declaration of SceneEditData

class SceneController : public Observer, public Observable {
public:
    SceneController(std::filesystem::path tomlFilePath, ApplicationState* appState);
    ~SceneController();

    void onNotify(const UserEvent event) override;

    void applyScene(const std::string& sceneName);

    void loadRandomScene();
    void autoSwitchScenes(uint64_t timeInSeconds);

    void createScene(std::string sceneName) {
        scenes_.try_emplace(sceneName, sceneName);
    }

    void deleteScene(const std::string& sceneName) {
        scenes_.erase(sceneName);
    }

    void editScene(const SceneEditData& editData);

private:
    void loadFromToml();
    bool saveToToml();

    bool isActiveTrailMaskAnimating() const;

    std::filesystem::path tomlFilePath_;
    ApplicationState* appState_;
    std::map<std::string, Scene> scenes_;
    std::string activeScene_;
};

#endif // SCENE_CONTROLLER_H