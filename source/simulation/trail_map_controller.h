#ifndef TRAIL_MAP_CONTROLLER_H
#define TRAIL_MAP_CONTROLLER_H

#include <vector>
#include <string>

#include <GL/glew.h>
#include <SDL3/SDL.h>

#include "../graphics/texture.h"


class TrailMapController {
public:

    TrailMapController();
    void loadTrailMaskFromImage(const std::string& imagePath, GLuint textureUnit);
    GLuint getTrailMaskTextureID() const { return trailMaskTexture_; }

private:
    SDL_Surface* loadedImage_;

    GLuint trailMaskTexture_ = 0;
};

#endif // TRAIL_MAP_CONTROLLER_H