#include "trail_map_controller.h"

#include <ctime>
#include <fstream>
#include <iostream>

#include <SDL3_image/SDL_image.h>
#include <toml.hpp>

#include "../application_state.h"
#include "../ui/elements/preset_window.h"
#include "../utility/event.h"
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
 : pictureFilePath_(pictureFilePath), 
   pictureFileExtension_(pictureFileExtension), 
   textureUnit_(textureUnit),
   fontAtlas_{FontAtlas("Roboto-Medium")}
{
    loadPictureNames(ui);
    for(size_t i = 0; i < trailMasks_.size(); ++i) {
        activeTrailMaskIndex_ = i;
        loadTrailMaskFromImage(trailMasks_[i].imageName);
    }
    ApplicationState* appState = ui->getState();

    textImage_ = TextTexture(appState->universalShaderSettings.textureWidth, appState->universalShaderSettings.textureHeight, appState);
    textImage_.createTexture(appState->textPreset, fontAtlas_);
    textImage_.textureToFile();
    trailMasks_[activeTrailMaskIndex_].texture = std::move(textImage_);

    activeTrailMaskIndex_ = 0;	//reset to first image after loading all images into GPU memory
}
/*
void TrailMapController::loadTrailMaskFromFont(std::string fontName) {

}
*/
void TrailMapController::loadTrailMaskFromImage(std::string imageName) {

    SDL_Surface* loadedImage = loadImageFromFile(pictureFilePath_, imageName, pictureFileExtension_);
    loadImageFromSurface(loadedImage);
}

void TrailMapController::loadImageFromSurface(SDL_Surface* surface) {
    if(surface == nullptr) {
        return; //error message already printed in loadImageFromFile
    }

    TextureProperties properties;
    properties.width = surface->w;
    properties.height = surface->h;
    properties.wrapX = TextureWrap::CLAMP_TO_BORDER;
    properties.wrapY = TextureWrap::CLAMP_TO_BORDER;
    properties.minFilter = TextureMinFilter::LINEAR;
    properties.magFilter = TextureMagFilter::LINEAR;
    properties.generateMipmaps = false;

    Texture tempTexture(properties, surface->pixels, TextureDataFormat::RGBA, TextureDataType::UBYTE, surface->pitch);
    
    trailMasks_[activeTrailMaskIndex_].texture = std::move(tempTexture);
    trailMasks_[activeTrailMaskIndex_].loadedToGPU = true;

    SDL_DestroySurface(surface);
}

void TrailMapController::loadPictureNames(UserInterface* ui) {

    PresetWindow *window = dynamic_cast<PresetWindow*>(ui->getWindow("PresetWindow"));
    
    std::vector<std::string> pictureNames;

    getFileNamesInDirectory(pictureFilePath_, pictureFileExtension_, pictureNames);

    for (std::string pictureName : pictureNames) {
        window->addPictureName(pictureName);
        trailMasks_.push_back({pictureName, Texture(), false, false});
    }

    window->addPictureName("text");
    trailMasks_.push_back({"text", Texture(), true, false});
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

    ApplicationState* appState = ui->getState();

    //Timed Auto Preset Switching
    if(appState->autoPresetSwitching) {
        if((timeInSeconds % (Uint64)appState->trailMaskIntervall == 0) && !timeOut_ && appState->slimeSettings.velocityBassReaction > appState->beatVolumeSwitch) {
            loadRandomPicture(ui);
            timeOut_ = true;
        } else if((timeInSeconds % (Uint64)appState->trailMaskIntervall > 0) && timeOut_){
            timeOut_ = false;
        }
    }
}

void TrailMapController::bindToTextureUnit(GLuint textureUnit) { 
    textureUnit_ = textureUnit;
    glActiveTexture(GL_TEXTURE0 + textureUnit_);
    glBindTexture(GL_TEXTURE_2D, trailMasks_[activeTrailMaskIndex_].texture.getID());
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
                        glBindTexture(GL_TEXTURE_2D, trailMasks_[i].texture.getID());
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
