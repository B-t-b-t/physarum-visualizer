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
    for(const TrailMask& trailMask : trailMasks_) {
        if(trailMask.imageName != imageName) {
            continue;
        }

        //trail masks without a time slot are always valid
        if(!trailMask.hasTimeSlot) {
            return true;
        }

        SDL_Time currentTime{};
        if(!SDL_GetCurrentTime(&currentTime)) {
            std::cerr << "Failed to get the current time: "
                      << SDL_GetError() << std::endl;
            return false;
        }

        return currentTime >= trailMask.beginTimeSlot &&
               currentTime <= trailMask.endTimeSlot;
    }

    //a trail mask not present in the list cannot be selected.
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

    for(TrailMask& trailMask : trailMasks_) {
        if(!timeTable_.contains(trailMask.imageName)) {
            continue;
        }

        const auto& entry = toml::find(timeTable_, trailMask.imageName);
        if(!entry.contains("begin") || !entry.contains("end")) {
            continue;
        }

        const auto beginDateTime = toml::find<toml::local_datetime>(entry, "begin");
        const auto endDateTime = toml::find<toml::local_datetime>(entry, "end");

        trailMask.hasTimeSlot = true;
        trailMask.beginTimeSlot =
            static_cast<SDL_Time>(beginDateTime.operator time_t()) * 1000000000LL;
        trailMask.endTimeSlot =
            static_cast<SDL_Time>(endDateTime.operator time_t()) * 1000000000LL;
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
            SDL_DateTime beginDateTime{};
            SDL_DateTime endDateTime{};

            if(!SDL_TimeToDateTime(trailMask.beginTimeSlot, &beginDateTime, true) ||
               !SDL_TimeToDateTime(trailMask.endTimeSlot, &endDateTime, true)) {
                std::cerr << "Failed to convert trail mask time slot for \""
                          << trailMask.imageName << "\": " << SDL_GetError() << std::endl;
                return false;
            }

            entry["end"] = toml::local_datetime{
                toml::local_date{
                    static_cast<int>(endDateTime.year),
                    static_cast<toml::month_t>(endDateTime.month - 1),    //-1 because SDL3 months start at 1
                    static_cast<int>(endDateTime.day)
                },
                toml::local_time{
                    endDateTime.hour,
                    endDateTime.minute,
                    endDateTime.second,
                    0,
                    0,
                    0
                }
            };

            entry["begin"] = toml::local_datetime{
                toml::local_date{
                    static_cast<int>(beginDateTime.year),
                    static_cast<toml::month_t>(beginDateTime.month - 1),    //-1 because SDL3 months start at 1
                    static_cast<int>(beginDateTime.day)
                },
                toml::local_time{
                    beginDateTime.hour,
                    beginDateTime.minute,
                    beginDateTime.second,
                    0,
                    0,
                    0
                }
            };
        }

        newTimeTable[trailMask.imageName] = entry;
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

void TrailMapController::editTextTrailMask(int index, TrailMaskData newData) {
    appState_ = appState_;

    if(newData.hasTimeSlot) {
        SDL_Time currentTime;
        SDL_DateTime currentDateTime;
        SDL_GetCurrentTime(&currentTime);
        SDL_TimeToDateTime(currentTime, &currentDateTime, true);

        //!!! quick and dirty, breaks with month/year changes!!!
        SDL_DateTime beginTimeSlot = currentDateTime;
        SDL_DateTime endTimeSlot = currentDateTime;

        beginTimeSlot.day = newData.dayBegin;
        beginTimeSlot.hour = newData.hourBegin;
        beginTimeSlot.minute = newData.minuteBegin;
        endTimeSlot.day = newData.dayEnd;
        endTimeSlot.hour = newData.hourEnd;
        endTimeSlot.minute = newData.minuteEnd;

        SDL_Time beginTimeSlotTicks;
        SDL_Time endTimeSlotTicks;

        SDL_DateTimeToTime(&beginTimeSlot, &beginTimeSlotTicks);
        SDL_DateTimeToTime(&endTimeSlot, &endTimeSlotTicks);

        trailMasks_[(size_t)index].hasTimeSlot = true;
        trailMasks_[(size_t)index].beginTimeSlot = beginTimeSlotTicks;
        trailMasks_[(size_t)index].endTimeSlot = endTimeSlotTicks;
    }

    if(index >= 0 && (size_t)index < trailMasks_.size()) {
        ((TextTexture*)trailMasks_[(size_t)index].texture.get())->createTexture(newData.newName, fontAtlas_);
        trailMasks_[(size_t)index].imageName = newData.newName;
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

void TrailMapController::editTrailMaskTimeSlot(int index, const TrailMaskData& newData) {
    if(index < 0 || static_cast<size_t>(index) >= trailMasks_.size()) {
        return;
    }

    TrailMask& trailMask = trailMasks_[(size_t)index];
    const bool hadTimeSlot = trailMask.hasTimeSlot;

    trailMask.hasTimeSlot = newData.hasTimeSlot;

    if(!trailMask.hasTimeSlot) {
        trailMask.beginTimeSlot = {};
        trailMask.endTimeSlot = {};
        return;
    }

    SDL_Time currentTime{};
    SDL_DateTime currentDateTime{};

    if(!SDL_GetCurrentTime(&currentTime) ||
       !SDL_TimeToDateTime(currentTime, &currentDateTime, true)) {
        std::cerr << "Failed to get the current date and time: "
                  << SDL_GetError() << std::endl;
        trailMask.hasTimeSlot = false;
        return;
    }

    SDL_DateTime beginDateTime = currentDateTime;
    SDL_DateTime endDateTime = currentDateTime;

    // Preserve the existing date when modifying an existing time slot.
    if(hadTimeSlot) {
        SDL_TimeToDateTime(trailMask.beginTimeSlot, &beginDateTime, true);
        SDL_TimeToDateTime(trailMask.endTimeSlot, &endDateTime, true);
    }

    beginDateTime.day = newData.dayBegin;
    beginDateTime.hour = newData.hourBegin;
    beginDateTime.minute = newData.minuteBegin;
    beginDateTime.second = 0;
    beginDateTime.nanosecond = 0;

    endDateTime.day = newData.dayEnd;
    endDateTime.hour = newData.hourEnd;
    endDateTime.minute = newData.minuteEnd;
    endDateTime.second = 0;
    endDateTime.nanosecond = 0;

    SDL_Time beginTimeSlot{};
    SDL_Time endTimeSlot{};

    if(!SDL_DateTimeToTime(&beginDateTime, &beginTimeSlot) ||
       !SDL_DateTimeToTime(&endDateTime, &endTimeSlot)) {
        std::cerr << "Failed to create trail mask time slot: "
                  << SDL_GetError() << std::endl;
        trailMask.hasTimeSlot = false;
        return;
    }

    trailMask.beginTimeSlot = beginTimeSlot;
    trailMask.endTimeSlot = endTimeSlot;
}

void TrailMapController::onNotify(const UserEvent event) {

    switch (event.type) {
        case EventType::LOAD_NEW_PICTURE:
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
        case EventType::CREATE_NEW_TEXT_TEXTURE:
        {
            loadTrailMaskFromText(std::get<std::string>(event.data_1));
            break;
        }
        case EventType::EDIT_TEXT_TEXTURE:
        {
            editTextTrailMask(std::get<int>(event.data_1), std::get<TrailMaskData>(event.data_2));
            break;
        }
        case EventType::DELETE_TEXT_TEXTURE:
        {
            deleteTrailMask((size_t)std::get<int>(event.data_1));
            break;
        }
        case EventType::EDIT_TRAIL_MASK_TIME_SLOT:
        {
            editTrailMaskTimeSlot(
                std::get<int>(event.data_1),
                std::get<TrailMaskData>(event.data_2)
            );
            break;
        }
        default:
            break;
    }
}
