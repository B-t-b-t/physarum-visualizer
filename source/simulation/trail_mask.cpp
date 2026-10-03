#include "trail_mask.h"

#include <memory>

#include "../application_state.h"
#include "../graphics/font_atlas.h"
#include "../graphics/texture.h"
#include "../graphics/text_texture.h"

TrailMask::TrailMask(std::string nameIn, TrailMaskType typeIn, TrailMaskProperties propertiesIn)
    : name{nameIn}, type{typeIn}
{
    text = propertiesIn.text;
    if(propertiesIn.timeSlot) { timeSlot = propertiesIn.timeSlot; }
    strength = propertiesIn.strength;
    position = propertiesIn.position;
    scale = propertiesIn.scale;
    isInverted = propertiesIn.isInverted;
}

TrailMask::TrailMask(const TrailMask& other) {
    name = other.name;
    type = other.type;
    if(!other.text.empty()) { text = other.text; }
    if(other.texture) { texture = std::make_unique<Texture>(std::move(*other.texture)); }  //don't overwrite with nullptr
    timeSlot = other.timeSlot;
    strength = other.strength;
    position = other.position;
    scale = other.scale;
    aspectRatioCorrection_ = other.aspectRatioCorrection_;
    isInverted = other.isInverted;
}

TrailMask& TrailMask::operator=(const TrailMask& other) {
    if (this != &other) {
        name = other.name;
        type = other.type;
        if(!other.text.empty()) { text = other.text; }
        if(other.texture) { texture = std::make_unique<Texture>(std::move(*other.texture)); }  //don't overwrite with nullptr
        timeSlot = other.timeSlot;
        strength = other.strength;
        position = other.position;
        scale = other.scale;
        aspectRatioCorrection_ = other.aspectRatioCorrection_;
        isInverted = other.isInverted;
    }
    return *this;
}

TrailMask::TrailMask(TrailMask&& other) {
    name = std::move(other.name);
    type = other.type;
    text = std::move(other.text);
    texture = std::move(other.texture);
    timeSlot = std::move(other.timeSlot);
    strength = std::move(other.strength);
    position = std::move(other.position);
    scale = std::move(other.scale);
    aspectRatioCorrection_ = std::move(other.aspectRatioCorrection_);
    isInverted = std::move(other.isInverted);
}

TrailMask& TrailMask::operator=(TrailMask&& other) {
    if (this != &other) {
        name = std::move(other.name);
        type = other.type;
        text = std::move(other.text);
        texture = std::move(other.texture);
        timeSlot = std::move(other.timeSlot);
        strength = std::move(other.strength);
        position = std::move(other.position);
        scale = std::move(other.scale);
        aspectRatioCorrection_ = std::move(other.aspectRatioCorrection_);
        isInverted = std::move(other.isInverted);
    }
    return *this;
}

void TrailMask::createTextureFromImage(std::string& pictureFilePath, std::string& imageName, std::string& pictureFileExtension, ApplicationState* appState) {
    if(type == TrailMaskType::IMAGE) {
        SDL_Surface* loadedImage = loadImageFromFile(pictureFilePath, imageName, pictureFileExtension);

        TextureProperties properties;
        properties.width = loadedImage->w;
        properties.height = loadedImage->h;
        properties.wrapX = TextureWrap::CLAMP_TO_BORDER;
        properties.wrapY = TextureWrap::CLAMP_TO_BORDER;
        properties.minFilter = TextureMinFilter::LINEAR;
        properties.magFilter = TextureMagFilter::LINEAR;
        properties.generateMipmaps = false;

        Texture tempTexture(properties, loadedImage->pixels, TextureDataFormat::RGBA, TextureDataType::UBYTE, loadedImage->pitch);
        
        texture = std::make_unique<Texture>(std::move(tempTexture));

        SDL_DestroySurface(loadedImage);

        int texWidth = texture->getWidth();
        int texHeight = texture->getHeight();
        float sizeRatio = texWidth / (float) texHeight;
        float canvasRatio = appState->universalShaderSettings.textureWidth / (float) appState->universalShaderSettings.textureHeight;

        if(sizeRatio > 1.0f) {     //wider than tall
            if(sizeRatio > canvasRatio) {
                aspectRatioCorrection_.y *= canvasRatio / sizeRatio;
            } else {
                aspectRatioCorrection_.x *= sizeRatio / canvasRatio;
            }
        } else if(sizeRatio < 1.0f) {   // taller than wide
            aspectRatioCorrection_.x *= sizeRatio / canvasRatio;
        }
    }
}

void TrailMask::createTextureFromText(std::string& textIn, FontAtlas& fontAtlas, ApplicationState* appState) {
    if(type == TrailMaskType::TEXT) {
        text = textIn;
        auto textTexture = std::make_unique<TextTexture>(appState->universalShaderSettings.textureWidth, appState->universalShaderSettings.textureHeight, appState);

        textTexture->createTexture(text, fontAtlas);
        texture = std::move(textTexture);
    }
}

std::string TrailMask::makeKey(const TrailMaskType type, const std::string& name) {
    std::string key = "";

    switch (type) {
        case TrailMaskType::IMAGE:
            key = "Image." + name;
            break;
        case TrailMaskType::TEXT:
            key = "Text." + name;
            break;
    }
    
    return key;
}