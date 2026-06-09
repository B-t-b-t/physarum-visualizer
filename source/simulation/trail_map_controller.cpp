#include "trail_map_controller.h"

#include <ctime>
#include <fstream>
#include <iostream>

#include <SDL3_image/SDL_image.h>
#include <toml.hpp>

#include "../ui/elements/preset_window.h"
#include "../utility/fileHandling.h"

bool TrailMapController::checkTimeTable(std::string imageName) {

    const toml::value timeTable = toml::parse("./res/pictures/timeTable.toml");

    if(!timeTable.contains(imageName)) {
        return true;    //if no entry for the image is found, it is always valid to use
    }

    const auto key = toml::find(timeTable, imageName);
    const auto begin_t   = toml::find<toml::local_datetime>(key, "begin");
    const auto end_t     = toml::find<toml::local_datetime>(key, "end");

    SDL_Time begin = begin_t.operator time_t() * 1000000000;    //convert to nanoseconds for SDL3
    SDL_Time end = end_t.operator time_t() * 1000000000;

    //compare if current time is within parsed time window
    SDL_Time current;
    SDL_GetCurrentTime(&current);

    return (current >= begin && current <= end);
}


TrailMapController::TrailMapController(std::string pictureFilePath, std::string pictureFileExtension, GLuint textureUnit, UserInterface* ui)
 : pictureFilePath_(pictureFilePath), pictureFileExtension_(pictureFileExtension), textureUnit_(textureUnit) {
    loadPictureNames(ui);
    for(size_t i = 0; i < trailMasks_.size(); ++i) {
        activeTrailMaskIndex_ = i;
        loadTrailMaskFromImage(trailMasks_[i].imageName);
    }
    activeTrailMaskIndex_ = 0;	//reset to first image after loading all images into GPU memory
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
    glBindTexture(GL_TEXTURE_2D, trailMaskTextureID);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);  //to avoid repetition when scaling
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);  //border color is default (0,0,0,0)

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

void TrailMapController::loadPictureNames(UserInterface* ui) {

    PresetWindow *window = dynamic_cast<PresetWindow*>(ui->getWindow("PresetWindow"));
    
    std::vector<std::string> pictureNames;

    loadFileNames(pictureFilePath_, pictureFileExtension_, pictureNames);

    for (std::string pictureName : pictureNames) {
        window->addPictureName(pictureName);
        trailMasks_.push_back({pictureName, 0, false});
    }
}

/*Loads Images indirectly, where the selection in the ListBox of the window is set and a call to handleUIRequests is made later in main()
    !UGLY and confusing, please rewrite!!
*/
void TrailMapController::loadRandomPicture(UserInterface* ui) {
    if(!trailMasks_.empty()) {
        
        SDL_GetCurrentTime(&timeTicks_);
        SDL_TimeToDateTime(timeTicks_, &dateTime_, true);
        
        unsigned int randomIndex = (unsigned int) (rand() % (int)trailMasks_.size());
        std::string imageName = trailMasks_[randomIndex].imageName;
        
        while(!checkTimeTable(imageName)) {
            randomIndex = (unsigned int) (rand() % (int)trailMasks_.size());
            imageName = trailMasks_[randomIndex].imageName;
        }
        
        PresetWindow *window = dynamic_cast<PresetWindow*>(ui->getWindow("PresetWindow"));
        window->setSelectedPicture(randomIndex);
        activeTrailMaskIndex_ = randomIndex;
        
    } else {
        std::cerr << "WARN: No pictures available to auto switch" << std::endl;
    }
}

void TrailMapController::autoSwitchPictures(UserInterface* ui, Uint64 timeInSeconds) {

    UIState* uiState = ui->getState();

    //Timed Auto Preset Switching
    if(uiState->autoPresetSwitching) {
        if((timeInSeconds % (Uint64)uiState->trailMaskIntervall == 0) && !timeOut_ && uiState->slimeSettings.velocityBassReaction > uiState->beatVolumeSwitch) {
            loadRandomPicture(ui);
            timeOut_ = true;
        } else if((timeInSeconds % (Uint64)uiState->trailMaskIntervall > 0) && timeOut_){
            timeOut_ = false;
        }
    }
}

void TrailMapController::bindToTextureUnit(GLuint textureUnit) { 
    textureUnit_ = textureUnit;
    glActiveTexture(GL_TEXTURE0 + textureUnit_);
    glBindTexture(GL_TEXTURE_2D, trailMasks_[activeTrailMaskIndex_].textureID);
}

void TrailMapController::onNotify(const Event event) {
    PresetWindow *window = dynamic_cast<PresetWindow*>(observable_);

    switch (event) {
        case Event::LOAD_NEW_PICTURE:
        {
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
            break;
        }
        default:
            break;
    }
}
