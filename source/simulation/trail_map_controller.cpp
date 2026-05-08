#include "trail_map_controller.h"

#include <iostream>

#include <SDL3_image/SDL_image.h>

TrailMapController::TrailMapController() {
    // Constructor can be used for initialization if needed
}

void TrailMapController::loadTrailMaskFromImage(const std::string& imagePath, GLuint textureUnit) {
    SDL_Surface* surface = IMG_Load(imagePath.c_str());
    if(surface == nullptr) {
        std::cerr << "ERROR: Failed to load image: " << SDL_GetError() << std::endl;
        return;
    }

    SDL_Surface* formattedSurface = SDL_ConvertSurface(surface, SDL_PIXELFORMAT_RGBA32);
    SDL_DestroySurface(surface);

    if(formattedSurface == nullptr) {
        std::cerr << "ERROR: Failed to convert surface format: " << SDL_GetError() << std::endl;
        return;
    }

    if(!SDL_FlipSurface(formattedSurface, SDL_FLIP_VERTICAL)) { //flip vertically for OpenGL coordinate system
        std::cerr << "ERROR: Failed to flip texture: " << SDL_GetError() << std::endl;
        SDL_DestroySurface(formattedSurface);
        return;
    }

    glGenTextures(1, &trailMaskTexture_);
    glActiveTexture(GL_TEXTURE0 + textureUnit);
    glBindTexture(GL_TEXTURE_2D, trailMaskTexture_);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glPixelStorei(GL_UNPACK_ROW_LENGTH, formattedSurface->pitch / 4);

    glTexImage2D(
        GL_TEXTURE_2D,
        0,
        GL_RGBA32F,
        formattedSurface->w,
        formattedSurface->h,
        0,
        GL_RGBA,
        GL_UNSIGNED_BYTE,
        formattedSurface->pixels
    );

    glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
    glBindTexture(GL_TEXTURE_2D, 0);

    SDL_DestroySurface(formattedSurface);
}