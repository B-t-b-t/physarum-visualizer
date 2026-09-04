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


TrailMapController::TrailMapController(std::string pictureFilePath, std::string pictureFileExtension, GLuint textureUnit, ApplicationState* appState)
 : pictureFilePath_(pictureFilePath), 
   pictureFileExtension_(pictureFileExtension), 
   textureUnit_(textureUnit),
   appState_(appState),
   fontAtlas_{FontAtlas("Roboto-Medium")}
{
    loadPictureNames();
    for(size_t i = 0; i < trailMasks_.size(); ++i) {
        activeTrailMaskIndex_ = i;
        loadTrailMaskFromImage(trailMasks_[i].imageName);
    }

    //textImage_ = TextTexture(appState_->universalShaderSettings.textureWidth, appState_->universalShaderSettings.textureHeight, appState_);
    //textImage_.createTexture(appState_->textPreset, fontAtlas_);
    //textImage_.textureToFile();
    //trailMasks_[activeTrailMaskIndex_].texture = std::move(textImage_);

    activeTrailMaskIndex_ = 0;	//reset to first image after loading all images into GPU memory

    appState_->trailMasks = &trailMasks_;
    appState_->usedTrailMaskIndex = activeTrailMaskIndex_;
}

void TrailMapController::loadTrailMaskFromText(std::string text) {
    textImage_ = TextTexture(appState_->universalShaderSettings.textureWidth, appState_->universalShaderSettings.textureHeight, appState_);
    textImage_.createTexture(text, fontAtlas_);
    //textImage_.textureToFile();
    trailMasks_.push_back({text, std::move(textImage_), true, true});
}

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

void TrailMapController::loadPictureNames() {
    
    std::vector<std::string> pictureNames;

    getFileNamesInDirectory(pictureFilePath_, pictureFileExtension_, pictureNames);

    for (std::string pictureName : pictureNames) {
        trailMasks_.push_back({pictureName, Texture(), false, false});
    }

    //trailMasks_.push_back({"text", Texture(), true, false});
}

/*Loads Images indirectly, where the selection in the ListBox of the window is set and a call to handleUIRequests is made later in main()
    !UGLY and confusing, please rewrite!!
*/
void TrailMapController::loadRandomPicture() {
    if(!trailMasks_.empty()) {
        
        SDL_GetCurrentTime(&timeTicks_);
        SDL_TimeToDateTime(timeTicks_, &dateTime_, true);
        
        unsigned int randomIndex = 0;
        std::string imageName = "";
        
        // try until a random picture passes the timetable check
        do {   
            randomIndex = (unsigned int) (rand() % (int)trailMasks_.size());
            imageName = trailMasks_[randomIndex].imageName;
        } while(!checkTimeTable(imageName));
        
        activeTrailMaskIndex_ = randomIndex;
        appState_->usedTrailMaskIndex = randomIndex;
        
    } else {
        std::cerr << "WARN: No pictures available to auto switch" << std::endl;
    }
}

void TrailMapController::autoSwitchPictures(Uint64 timeInSeconds) {
    //Timed Auto Preset Switching
    if(appState_->autoPresetSwitching) {
        if((timeInSeconds % (Uint64)appState_->trailMaskIntervall == 0) && !timeOut_ && appState_->slimeSettings.velocityBassReaction > appState_->beatVolumeSwitch) {
            loadRandomPicture();
            timeOut_ = true;
        } else if((timeInSeconds % (Uint64)appState_->trailMaskIntervall > 0) && timeOut_){
            timeOut_ = false;
        }
    }
}

void TrailMapController::bindToTextureUnit(GLuint textureUnit) { 
    textureUnit_ = textureUnit;
    glActiveTexture(GL_TEXTURE0 + textureUnit_);
    glBindTexture(GL_TEXTURE_2D, trailMasks_[activeTrailMaskIndex_].texture.getID());
}

void TrailMapController::deleteTrailMask(size_t index) {

    //copy all Trail Masks to member vector except the one to be deleted
    std::vector<TrailMask> tempMasks = std::move(trailMasks_);
    trailMasks_.clear();
    trailMasks_.reserve(tempMasks.size() - 1);

    for(size_t i = 0; i < tempMasks.size(); ++i) {
        if(i != index) {
            trailMasks_.push_back(std::move(tempMasks[i]));
        }
    }

    //ensure activeTrailMaskIndex_ is within bounds after deletion
    if(index < activeTrailMaskIndex_) {
        --activeTrailMaskIndex_;
    } else if(index == activeTrailMaskIndex_) {
        activeTrailMaskIndex_ = 0;
    }

    appState_->usedTrailMaskIndex = activeTrailMaskIndex_;
}

void TrailMapController::onNotify(const UserEvent event) {

    switch (event.type) {
        case EventType::LOAD_NEW_PICTURE:
        {
            activeTrailMaskIndex_ = appState_->usedTrailMaskIndex;

            if(trailMasks_[activeTrailMaskIndex_].loadedToGPU) {
                glActiveTexture(GL_TEXTURE0 + textureUnit_);
                glBindTexture(GL_TEXTURE_2D, trailMasks_[activeTrailMaskIndex_].texture.getID());
            } else {
                loadTrailMaskFromImage(trailMasks_[activeTrailMaskIndex_].imageName);
            }
            break;
        }
        case EventType::CREATE_NEW_TEXT_TEXTURE:
        {
            loadTrailMaskFromText(appState_->textPreset);
            break;
        }
        case EventType::EDIT_TEXT_TEXTURE:
        {
            //editTrailMaskFromText(appState_->textPreset);
            break;
        }
        case EventType::DELETE_TEXT_TEXTURE:
        {
            deleteTrailMask(appState_->usedTrailMaskIndex);
            break;
        }
        default:
            break;
    }
}
