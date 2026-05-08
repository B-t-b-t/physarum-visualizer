#include "trail_map_controller.h"

#include <iostream>

#include <SDL3_image/SDL_image.h>

#include "../ui/elements/preset_window.h"
#include "../utility/fileHandling.h"

TrailMapController::TrailMapController(std::string pictureFilePath, std::string pictureFileExtension, GLuint textureUnit) : pictureFilePath_(pictureFilePath), pictureFileExtension_(pictureFileExtension), textureUnit_(textureUnit) {
    // Constructor can be used for initialization if needed
}

void TrailMapController::loadTrailMaskFromImage(std::string imageName) {
    SDL_Surface* surface = IMG_Load((pictureFilePath_ + imageName + pictureFileExtension_).c_str());
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

    GLuint trailMaskTextureID;
    glGenTextures(1, &trailMaskTextureID);
    //glActiveTexture(GL_TEXTURE0 + textureUnit);
    glBindTexture(GL_TEXTURE_2D, trailMaskTextureID);

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

    trailMasks_[activeTrailMaskIndex_].textureID = trailMaskTextureID;
    trailMasks_[activeTrailMaskIndex_].loadedToGPU = true;
}

void TrailMapController::loadPictureNames(UserInterface &ui) {

    PresetWindow *window = dynamic_cast<PresetWindow*>(ui.getWindow("PresetWindow"));
    
    std::vector<std::string> pictureNames;

    loadFileNames(pictureFilePath_, pictureFileExtension_, pictureNames);

    for (std::string &pictureName : pictureNames) {
        window->addPictureName(pictureName);
        trailMasks_.push_back({pictureName, 0, false});
    }
}

/*Loads Images indirectly, where the selection in the ListBox of the window is set and a call to handleUIRequests is made later in main()
    !UGLY and confusing, please rewrite!!
*/
void TrailMapController::loadRandomPicture(UserInterface &ui) {
    PresetWindow *window = dynamic_cast<PresetWindow*>(ui.getWindow("PresetWindow"));

    unsigned int randomIndex = (unsigned int) (rand() % (int)trailMasks_.size());
    window->setSelectedPicture(randomIndex);
}

void TrailMapController::handleUIRequests(UserInterface &ui) {
    UIState &uiState = ui.getState();
    PresetWindow *window = dynamic_cast<PresetWindow*>(ui.getWindow("PresetWindow"));

    //Loading Picture from File
    if(uiState.loadNewPicture) {
        std::string pictureName = std::string(window->getSelectedPictureName());

        //binds selected image or loads it into GPU memory if not already loaded
        for(size_t i = 0; i < trailMasks_.size(); ++i) {
            if(trailMasks_[i].imageName == pictureName) {
                activeTrailMaskIndex_ = i;

                if(trailMasks_[i].loadedToGPU) {
                    glActiveTexture(GL_TEXTURE0 + textureUnit_);
                    glBindTexture(GL_TEXTURE_2D, trailMasks_[i].textureID);
                    break;
                } else {
                    loadTrailMaskFromImage(pictureName);
                    break;
                }
            }
        }

        uiState.loadNewPicture = false;
    }
}

void TrailMapController::autoSwitchPictures(UserInterface &UserInterface, Uint64 timeInSeconds) {
    UIState &uiState = UserInterface.getState();

    //Timed Auto Preset Switching
    if(uiState.autoPresetSwitching) {
        uiState.saveToPreset = false;
        uiState.loadFromPreset = false;

        if((timeInSeconds % (Uint64)uiState.presetIntervall == 0) && !timeOut_ && uiState.slimeSettings.velocityBassReaction > uiState.beatVolumeSwitch) {
            loadRandomPicture(UserInterface);
            timeOut_ = true;
        } else if((timeInSeconds % (Uint64)uiState.presetIntervall > 0) && timeOut_){
            timeOut_ = false;
        }
    }
}

void TrailMapController::bindToTextureUnit(GLuint textureUnit) { 
    textureUnit_ = textureUnit;
    glActiveTexture(GL_TEXTURE0 + textureUnit_);
    glBindTexture(GL_TEXTURE_2D, trailMasks_[activeTrailMaskIndex_].textureID);
}
