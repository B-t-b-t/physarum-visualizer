#ifndef TRAIL_MAP_CONTROLLER_H
#define TRAIL_MAP_CONTROLLER_H

#include <chrono>
#include <map>
#include <memory>
#include <optional>
#include <string>

#include <GL/glew.h>
#include <SDL3/SDL.h>
#include <toml.hpp>

#include "../application_state.h"
#include "../graphics/font_atlas.h"
#include "../graphics/texture.h"
#include "../graphics/text_texture.h"
#include "../utility/event.h"
#include "../utility/observer.h"
#include "../utility/vector_math.h"

struct TrailMask {
        std::string name;
        TrailMaskType type;

        std::unique_ptr<Texture> texture{nullptr};

        std::optional<TimeSlot> timeSlot = std::nullopt;

        float strength{1.0f};
        phys::Vec2<float> position{0.0f, 0.0f};
        phys::Vec2<float> scale{1.0f, 1.0f};
};

class TrailMapController : public Observer {
public:

    TrailMapController() = default;
    TrailMapController(std::string pictureFilePath, std::string pictureFileExtension, GLuint textureUnit, ApplicationState* appState);
    TrailMapController(const TrailMapController&) = delete;
    TrailMapController& operator=(const TrailMapController&) = delete;
    TrailMapController(TrailMapController&&);
    TrailMapController& operator=(TrailMapController&&);
    ~TrailMapController();
    
    void loadTrailMaskFromImage(std::string imageName);
    void loadTrailMaskFromText(std::string text);
    void loadPictureNames();
    void bindToTextureUnit(GLuint textureUnit);

	void autoSwitchPictures(Uint64 timeInSeconds);
    void loadRandomPicture();
    void editTrailMask(const std::string& key, TrailMaskData newData);
    void deleteTrailMask(const std::string& key);
    void onNotify(const UserEvent event) override;

private:

    bool checkTimeTable(std::string name);
    bool loadFromToml();
    bool saveToToml();
    void loadImageFromSurface(const std::string& key, SDL_Surface* surface);

    SDL_Surface* loadedImage_;
    TextTexture textImage_;

    std::map<std::string, TrailMask> trailMasks_;

    std::string pictureFilePath_ = "./res/pictures/";
    std::string pictureFileExtension_ = ".png";
    GLuint textureUnit_;	//Default Texture Unit for Trail Mask Texture
    ApplicationState* appState_ = nullptr;

    FontAtlas fontAtlas_;

    std::string activeTrailMaskName_;
    float trailMaskStrengthTemp_ = 1.0f;
    SDL_Time timeTicks_;
    SDL_DateTime dateTime_;

    toml::value timeTable_{};

    bool timeOut_ = false;
};

#endif // TRAIL_MAP_CONTROLLER_H