#ifndef TRAIL_MAP_CONTROLLER_H
#define TRAIL_MAP_CONTROLLER_H

#include <vector>
#include <string>

#include <GL/glew.h>
#include <SDL3/SDL.h>

#include "../application_state.h"
#include "../graphics/font_atlas.h"
#include "../graphics/texture.h"
#include "../graphics/text_texture.h"
#include "../utility/observer.h"

struct TrailMask {
        std::string imageName;
        Texture texture;
        bool isText = false;
        bool loadedToGPU = false;
};

class TrailMapController : public Observer {
public:

    TrailMapController() = default;
    TrailMapController(std::string pictureFilePath, std::string pictureFileExtension, GLuint textureUnit, ApplicationState* appState);
    void loadTrailMaskFromImage(std::string imageName);
    void loadTrailMaskFromText(std::string text);
    void loadPictureNames();
    void bindToTextureUnit(GLuint textureUnit);

	void autoSwitchPictures(Uint64 timeInSeconds);
    void loadRandomPicture();
    void deleteTrailMask(size_t index);
    void onNotify(const UserEvent event) override;

private:

    bool checkTimeTable(std::string imageName);
    void loadImageFromSurface(SDL_Surface* surface);

    SDL_Surface* loadedImage_;
    TextTexture textImage_;

    std::vector<TrailMask> trailMasks_;

    std::string pictureFilePath_ = "./res/pictures/";
    std::string pictureFileExtension_ = ".png";
    GLuint textureUnit_;	//Default Texture Unit for Trail Mask Texture
    ApplicationState* appState_ = nullptr;

    FontAtlas fontAtlas_;

    size_t activeTrailMaskIndex_;
    SDL_Time timeTicks_;
    SDL_DateTime dateTime_;

    bool timeOut_ = false;
};

#endif // TRAIL_MAP_CONTROLLER_H