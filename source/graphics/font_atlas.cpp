#include "font_atlas.h"

#include "SDL3/SDL.h"

#include "texture.h"

FontAtlas::FontAtlas(std::string fontFileName) {

    SDL_Surface* loadedImage = loadImageFromFont("./res/fonts/", fontFileName, ".ttf", fontCharInfos_, &firstChar_, &numberOfChars_);
    saveImageToFile(loadedImage, "./", "fontAtlas", ".png", false);

    if(loadedImage == nullptr) {
        return; //error message already printed in loadImageFromFile
    }

    TextureProperties properties;
    properties.width = loadedImage->w;
    properties.height = loadedImage->h;
    properties.wrapX = TextureWrap::CLAMP_TO_BORDER;
    properties.wrapY = TextureWrap::CLAMP_TO_BORDER;
    properties.minFilter = TextureMinFilter::LINEAR;
    properties.magFilter = TextureMagFilter::LINEAR;
    properties.generateMipmaps = false;

    fontAtlas_ = Texture(properties, loadedImage->pixels, TextureDataFormat::RGBA, TextureDataType::UBYTE, loadedImage->pitch);
    
    SDL_DestroySurface(loadedImage);
}