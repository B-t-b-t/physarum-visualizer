#include "trail_map_controller.h"

#include <ctime>
#include <fstream>
#include <iostream>

#include <SDL3_image/SDL_image.h>
#include <toml.hpp>

#include "../application_state.h"
#include "../ui/windows/preset_window.h"
#include "../utility/event.h"
#include "../utility/fileHandling.h"

bool TrailMapController::checkTimeTable(std::string imageName) {
    for(const TrailMask& trailMask : trailMasks_) {
        if(trailMask.imageName != imageName) {
            continue;
        }

        //trail masks without a time slot are always valid
        if(!trailMask.hasTimeSlot) {
            return true;
        }

        auto currentTime = std::chrono::system_clock::now();
        return currentTime >= trailMask.timeSlotStart &&
               currentTime <= trailMask.timeSlotEnd;
    }

    //trail masks not present in the list cannot be selected.
    return false;
}

bool TrailMapController::loadFromToml() {
    //parse
    try {
        timeTable_ = toml::parse("./res/pictures/timeTable.toml");
    } catch(const toml::exception& err) {
        std::cerr << "Failed to parse timeTable.toml: " << err.what() << std::endl;
        return false;
    }

    //get all text trail masks without timeslots
    for(const auto& [entryName, entry] : timeTable_.as_table()) {
        if(!entry.contains("text")) {
            continue;
        }

        try {
            loadTrailMaskFromText(toml::find<std::string>(entry, "text"));
        } catch(const toml::exception& err) {
            std::cerr << "Failed to load text trail mask \"" << entryName
                      << "\": " << err.what() << std::endl;
        }
    }

    //load time slots for all trail masks, if they exist
    for(TrailMask& trailMask : trailMasks_) {
        std::string tableName = "";

        if(timeTable_.contains("Image." + trailMask.imageName) && !trailMask.isText) {
            tableName = "Image." + trailMask.imageName;
        } else if(timeTable_.contains("Text." + trailMask.imageName) && trailMask.isText) {
            tableName = "Text." + trailMask.imageName;
        } else {
            continue;
        }

        const auto& entry = toml::find(timeTable_, tableName);
        if(!entry.contains("begin") || !entry.contains("end")) {
            continue;
        }

        trailMask.hasTimeSlot = true;
        trailMask.timeSlotStart = toml::find<std::chrono::system_clock::time_point>(entry, "begin");
        trailMask.timeSlotEnd = toml::find<std::chrono::system_clock::time_point>(entry, "end");
    }

    return true;
}

bool TrailMapController::saveToToml() {
    toml::value newTimeTable{toml::table{}};

    for(const TrailMask& trailMask : trailMasks_) {
        //images without a time slot do not need an entry. Text masks must
        //always be persisted, even when they do not have a time slot.
        if(!trailMask.isText && !trailMask.hasTimeSlot) {
            continue;
        }

        toml::value entry{toml::table{}};

        if(trailMask.isText) {
            entry["text"] = trailMask.imageName;
        }

        if(trailMask.hasTimeSlot) {
            entry["end"] = toml::offset_datetime(trailMask.timeSlotEnd);
            entry["begin"] = toml::offset_datetime(trailMask.timeSlotStart);
        }

        if(trailMask.isText) {
            newTimeTable["Text." + trailMask.imageName] = entry;
        } else {
            newTimeTable["Image." + trailMask.imageName] = entry;
        }
    }

    std::ofstream outputFile{"./res/pictures/timeTable.toml", std::ios::trunc};
    if(!outputFile.is_open()) {
        std::cerr << "Failed to open timeTable.toml for writing" << std::endl;
        return false;
    }

    outputFile << toml::format(newTimeTable);
    outputFile.flush();

    if(!outputFile.good()) {
        std::cerr << "Failed to write timeTable.toml" << std::endl;
        return false;
    }

    timeTable_ = newTimeTable;
    return true;
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
    loadFromToml();

    activeTrailMaskIndex_ = 0;	//reset to first image after loading all images into GPU memory

    appState_->trailMasks = &trailMasks_;
    appState_->usedTrailMaskIndex = activeTrailMaskIndex_;
    trailMaskStrengthTemp_ = appState_->universalShaderSettings.trailMaskInfluence;
}

TrailMapController::TrailMapController(TrailMapController&& other) {
    pictureFilePath_ = std::move(other.pictureFilePath_);
    pictureFileExtension_ = std::move(other.pictureFileExtension_);
    textureUnit_ = other.textureUnit_;
    appState_ = other.appState_;
    fontAtlas_ = std::move(other.fontAtlas_);
    loadedImage_ = other.loadedImage_;
    textImage_ = std::move(other.textImage_);
    trailMasks_ = std::move(other.trailMasks_);
    activeTrailMaskIndex_ = other.activeTrailMaskIndex_;
    trailMaskStrengthTemp_ = other.trailMaskStrengthTemp_;
    timeTicks_ = other.timeTicks_;
    dateTime_ = other.dateTime_;
    timeTable_ = std::move(other.timeTable_);
    timeOut_ = other.timeOut_;

    other.loadedImage_ = nullptr;
    other.textureUnit_ = 0;
    other.appState_ = nullptr;
}

TrailMapController& TrailMapController::operator=(TrailMapController&& other) {
    if(this != &other) {
        pictureFilePath_ = std::move(other.pictureFilePath_);
        pictureFileExtension_ = std::move(other.pictureFileExtension_);
        textureUnit_ = other.textureUnit_;
        appState_ = other.appState_;
        fontAtlas_ = std::move(other.fontAtlas_);
        loadedImage_ = other.loadedImage_;
        textImage_ = std::move(other.textImage_);
        trailMasks_ = std::move(other.trailMasks_);
        activeTrailMaskIndex_ = other.activeTrailMaskIndex_;
        trailMaskStrengthTemp_ = other.trailMaskStrengthTemp_;
        timeTicks_ = other.timeTicks_;
        dateTime_ = other.dateTime_;
        timeTable_ = std::move(other.timeTable_);
        timeOut_ = other.timeOut_;

        other.loadedImage_ = nullptr;
        other.textureUnit_ = 0;
        other.appState_ = nullptr;
    }
    return *this;
}

TrailMapController::~TrailMapController() {
    saveToToml();
}

void TrailMapController::loadTrailMaskFromText(std::string text) {
    trailMasks_.push_back({text, std::make_unique<TextTexture>(appState_->universalShaderSettings.textureWidth, appState_->universalShaderSettings.textureHeight, appState_), true, true, false, {}, {}});
    ((TextTexture*)trailMasks_.back().texture.get())->createTexture(text, fontAtlas_);
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
    
    trailMasks_[activeTrailMaskIndex_].texture = std::make_unique<Texture>(std::move(tempTexture));
    trailMasks_[activeTrailMaskIndex_].loadedToGPU = true;

    SDL_DestroySurface(surface);
}

void TrailMapController::loadPictureNames() {
    
    std::vector<std::string> pictureNames;

    getFileNamesInDirectory(pictureFilePath_, pictureFileExtension_, pictureNames);

    Texture tempTexture = Texture();

    for (std::string pictureName : pictureNames) {
        trailMasks_.push_back({pictureName, std::make_unique<Texture>(std::move(tempTexture)), false, false, false, {}, {}});
    }
}

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
        
        //QUICK HACK: trailMasks get visually ugly, when the sensor distance is too large
        //reduce strength to 0 temporarily to allow slime behavior and colors to appear as intended
        if(appState_->slimeSettings.sensorDistance > appState_->disableAtSensorDistance) {
            //trailMaskStrengthTemp_ = appState_->universalShaderSettings.trailMaskInfluence;
            appState_->universalShaderSettings.trailMaskInfluence = 0.0f;
        } else {
            appState_->universalShaderSettings.trailMaskInfluence = trailMaskStrengthTemp_; //restore previous value
        }

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
    glBindTexture(GL_TEXTURE_2D, trailMasks_[activeTrailMaskIndex_].texture->getID());
}

void TrailMapController::editTextTrailMask(size_t index, TrailMaskData newData) {
    appState_ = appState_;

    if(newData.timeSlot.has_value()) {
        trailMasks_[index].hasTimeSlot = true;
        trailMasks_[index].timeSlotStart = newData.timeSlot.value().start;
        trailMasks_[index].timeSlotEnd = newData.timeSlot.value().end;
    } else {
        trailMasks_[index].hasTimeSlot = false;
        trailMasks_[index].timeSlotStart = {};
        trailMasks_[index].timeSlotEnd = {};
    }

    if(index < trailMasks_.size()) {
        ((TextTexture*)trailMasks_[index].texture.get())->createTexture(newData.name, fontAtlas_);
        trailMasks_[index].imageName = newData.name;
    }
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

void TrailMapController::editTrailMaskTimeSlot(size_t index, const TrailMaskData& newData) {
    if(index >= trailMasks_.size()) {
        return;
    }

    TrailMask& trailMask = trailMasks_[(size_t)index];

    if(newData.timeSlot.has_value()) {
        trailMask.hasTimeSlot = true;
        trailMask.timeSlotStart = newData.timeSlot.value().start;
        trailMask.timeSlotEnd = newData.timeSlot.value().end;
    } else {
        trailMask.hasTimeSlot = false;
        trailMask.timeSlotStart = {};
        trailMask.timeSlotEnd = {};
    }

    return;
}

void TrailMapController::onNotify(const UserEvent event) {

    switch (event.type) {
        case EventType::IMAGE_PRESET_APPLY:
        case EventType::TEXT_PRESET_APPLY: 
        {
            activeTrailMaskIndex_ = appState_->usedTrailMaskIndex;

            if(trailMasks_[activeTrailMaskIndex_].loadedToGPU) {
                glActiveTexture(GL_TEXTURE0 + textureUnit_);
                glBindTexture(GL_TEXTURE_2D, trailMasks_[activeTrailMaskIndex_].texture->getID());
            } else {
                loadTrailMaskFromImage(trailMasks_[activeTrailMaskIndex_].imageName);
            }
            break;
        }
        case EventType::TEXT_PRESET_CREATE:
        {
            loadTrailMaskFromText(std::get<std::string>(event.payload));
            break;
        }
        case EventType::TEXT_PRESET_EDIT:
        {
            TrailMaskData newData = std::get<TrailMaskData>(event.payload);
            if(newData.atIndex.has_value()) {
                editTextTrailMask(newData.atIndex.value(), newData);
            }
            break;
        }
        case EventType::TEXT_PRESET_DELETE:
        {
            deleteTrailMask((size_t)std::get<int>(event.payload));
            break;
        }
        case EventType::EDIT_TRAIL_MASK_TIME_SLOT:
        {
            TrailMaskData newData = std::get<TrailMaskData>(event.payload);
            if(newData.atIndex.has_value())
                editTrailMaskTimeSlot(newData.atIndex.value(), newData);
            break;
        }
        case EventType::TRAIL_MASK_STRENGTH_CHANGED:
        {
            if(appState_->universalShaderSettings.trailMaskInfluence > 0.0f) {
                trailMaskStrengthTemp_ = std::get<float>(event.payload);
            }
            break;
        }
        default:
            break;
    }
}
