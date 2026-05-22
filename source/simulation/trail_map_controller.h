#ifndef TRAIL_MAP_CONTROLLER_H
#define TRAIL_MAP_CONTROLLER_H

#include <vector>
#include <string>

#include <GL/glew.h>
#include <SDL3/SDL.h>

#include "../graphics/texture.h"
#include "../ui/user_interface.h"

class TrailMapController {
public:

    TrailMapController(std::string pictureFilePath, std::string pictureFileExtension, GLuint textureUnit, UserInterface &ui);
    void loadTrailMaskFromImage(std::string imageName);
    void loadPictureNames(UserInterface &ui);
    void bindToTextureUnit(GLuint textureUnit);

    void handleUIRequests(UserInterface &ui);
	void autoSwitchPictures(UserInterface &ui, Uint64 timeInSeconds);
    void loadRandomPicture(UserInterface &ui);

private:
    struct TrailMask {
        std::string imageName;
        GLuint textureID;
        bool loadedToGPU = false;
    };

    bool checkTimeTable(std::string imageName);


    SDL_Surface* loadedImage_;

    std::vector<TrailMask> trailMasks_;

    std::string pictureFilePath_ = "./res/pictures/";
    std::string pictureFileExtension_ = ".png";
    GLuint textureUnit_;	//Default Texture Unit for Trail Mask Texture
    UserInterface &ui_;

    size_t activeTrailMaskIndex_;
    SDL_Time timeTicks_;
    SDL_DateTime dateTime_;

    bool timeOut_ = false;
};

#endif // TRAIL_MAP_CONTROLLER_H