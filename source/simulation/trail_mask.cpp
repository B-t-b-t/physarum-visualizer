#include "trail_mask.h"

#include <filesystem>
#include <memory>

#include "../application_state.h"
#include "../graphics/font_atlas.h"
#include "../graphics/texture.h"
#include "../graphics/text_texture.h"
#include "../utility/filepaths.h"  // for FilePaths

TrailMask::TrailMask(std::string nameIn, TrailMaskType typeIn, TrailMaskProperties propertiesIn)
    : name{nameIn}, type{typeIn}
{
    text = propertiesIn.text;
    if(propertiesIn.timeSlot) { timeSlot = propertiesIn.timeSlot; }
    strength = propertiesIn.strength;
    position = propertiesIn.position;
    scale = propertiesIn.scale;

    if(propertiesIn.animation) { animation = propertiesIn.animation; }

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

    animation = other.animation;

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

        animation = other.animation;

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

    animation = std::move(other.animation);

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

        animation = std::move(other.animation);

        aspectRatioCorrection_ = std::move(other.aspectRatioCorrection_);
        isInverted = std::move(other.isInverted);
    }
    return *this;
}

void TrailMask::createTextureFromImage(std::filesystem::path pictureFilePath, ApplicationState* appState) {
    if(type == TrailMaskType::IMAGE) {
        SDL_Surface* loadedImage = loadImageFromFile(pictureFilePath);

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

void TrailMask::createTextureFromText(FontAtlas& fontAtlas, FilePaths* paths, ApplicationState* appState) {
    if(type == TrailMaskType::TEXT) {
        auto textTexture = std::make_unique<TextTexture>(appState->universalShaderSettings.textureWidth, appState->universalShaderSettings.textureHeight, paths, appState);

        textTexture->createTexture(text, fontAtlas);
        texture = std::move(textTexture);
    }
}

bool TrailMask::hasReachedDest(ApplicationState* appState) const {
    
    if(animation) {
        return animation.value().hasReachedDest(appState->universalShaderSettings.trailMaskPosition, phys::Vec2<float>{appState->universalShaderSettings.trailMaskScaleX, appState->universalShaderSettings.trailMaskScaleY});
    } else {
        return true;    //if there's no movement, it has "reached" its destination by default
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
        case TrailMaskType::EMPTY:
            key = "Empty." + name;
            break;
        //no default to force a compile error (if Werror=switch enabled)
    }
    
    return key;
}