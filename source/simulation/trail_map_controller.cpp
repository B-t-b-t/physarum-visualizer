#include "trail_map_controller.h"

#include <ctime>
#include <fstream>
#include <iostream>
#include <iterator>             // for next

#include <SDL3_image/SDL_image.h>
#include <toml.hpp>

#include "../application_state.h"
#include "../ui/windows/preset_window.h"
#include "../utility/event.h"
#include "../utility/fileHandling.h"

bool TrailMapController::checkTimeTable(std::string name) {
    for(const auto& [key, trailMask] : trailMasks_) {
        if(trailMask.name != name) {
            continue;
        }

        //trail masks without a time slot are always valid
        if(!trailMask.timeSlot) {
            return true;
        }

        auto currentTime = std::chrono::system_clock::now();
        return currentTime >= trailMask.timeSlot.value().start &&
               currentTime <= trailMask.timeSlot.value().end;
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

    //get all text trail masks
    for(const auto& [entryName, entry] : timeTable_.as_table()) {
        if(!entry.contains("text")) {   //discard entries without a "text" field
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
    for(auto& [key, trailMask] : trailMasks_) {
        std::string tableName = "";

        if(timeTable_.contains("Image." + trailMask.name) && trailMask.type == TrailMaskType::IMAGE) {
            tableName = "Image." + trailMask.name;
        } else if(timeTable_.contains("Text." + trailMask.name) && trailMask.type == TrailMaskType::TEXT) {
            tableName = "Text." + trailMask.name;
        } else {
            continue;
        }

        const auto& entry = toml::find(timeTable_, tableName);
        if(!entry.contains("begin") || !entry.contains("end")) {
            continue;
        }

        trailMask.timeSlot = TimeSlot{ 
            toml::find<std::chrono::system_clock::time_point>(entry, "begin"), toml::find<std::chrono::system_clock::time_point>(entry, "end")
        };
        try {
        trailMask.strength = toml::find_or<float>(entry, "strength", 1.0f);
        trailMask.position.x = toml::find_or<float>(entry, "position", 0, 0.0f);
        trailMask.position.y = toml::find_or<float>(entry, "position", 1, 0.0f);
        trailMask.scale.x = toml::find_or<float>(entry, "scale", 0, 1.0f);
        trailMask.scale.y = toml::find_or<float>(entry, "scale", 1, 1.0f);
        } catch(const toml::exception& err) {
            std::cerr << "Failed to load optional fields for trail mask \"" << trailMask.name
                      << "\": " << err.what() << std::endl;
            continue;
        }
    }

    return true;
}

bool TrailMapController::saveToToml() {
    toml::value newTimeTable{toml::table{}};

    for(const auto& [key, trailMask] : trailMasks_) {
        //images without a time slot do not need an entry. Text masks must
        //always be persisted, even when they do not have a time slot.
        if(trailMask.type == TrailMaskType::IMAGE && !trailMask.timeSlot) {
            continue;
        }

        //key value pairs are saved in reverse order from code
        toml::value entry{toml::table{}};

        float epsilon = 1e-6f;

        if(std::abs(trailMask.scale.x - 1.0f) > epsilon || std::abs(trailMask.scale.y - 1.0f) > epsilon) {
            entry["scale"] = toml::array{trailMask.scale.x, trailMask.scale.y};
        }

        if(std::abs(trailMask.position.x) > epsilon || std::abs(trailMask.position.y) > epsilon) {
            entry["position"] = toml::array{trailMask.position.x, trailMask.position.y};
        }

        if(std::abs(trailMask.strength - 1.0f) > epsilon) {
            entry["strength"] = trailMask.strength;
        }

        if(trailMask.timeSlot) {
            entry["end"] = toml::offset_datetime(trailMask.timeSlot.value().end);
            entry["begin"] = toml::offset_datetime(trailMask.timeSlot.value().start);
        }

        if(trailMask.type == TrailMaskType::TEXT) {
            entry["text"] = trailMask.name;
        }

        if(trailMask.type == TrailMaskType::TEXT) {
            newTimeTable[key] = entry;
        } else {
            newTimeTable[key] = entry;
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
    for(auto& [key, trailMask] : trailMasks_) {
        loadTrailMaskFromImage(trailMask.name);
    }
    loadFromToml();

    activeTrailMaskName_ = trailMasks_.empty() ? "" : trailMasks_.begin()->first;

    appState_->trailMasks = &trailMasks_;
    appState_->usedTrailMaskName = activeTrailMaskName_;
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
    activeTrailMaskName_ = other.activeTrailMaskName_;
    trailMaskStrengthTemp_ = other.trailMaskStrengthTemp_;
    timeTicks_ = other.timeTicks_;
    dateTime_ = other.dateTime_;
    timeTable_ = std::move(other.timeTable_);
    timeOut_ = other.timeOut_;

    //inform appState after move just in case
    if(appState_) {
        appState_->trailMasks = &trailMasks_;
        appState_->usedTrailMaskName = activeTrailMaskName_;
    }

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
        activeTrailMaskName_ = other.activeTrailMaskName_;
        trailMaskStrengthTemp_ = other.trailMaskStrengthTemp_;
        timeTicks_ = other.timeTicks_;
        dateTime_ = other.dateTime_;
        timeTable_ = std::move(other.timeTable_);
        timeOut_ = other.timeOut_;

        //inform appState after move just in case
        if(appState_) {
            appState_->trailMasks = &trailMasks_;
            appState_->usedTrailMaskName = activeTrailMaskName_;
        }

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
    std::string key = "Text." + text;

    auto [it, inserted] = trailMasks_.emplace(key, TrailMask{
        text, 
        TrailMaskType::TEXT,
        std::make_unique<TextTexture>(appState_->universalShaderSettings.textureWidth, appState_->universalShaderSettings.textureHeight, appState_), 
        true, 
        std::nullopt});

    if(!inserted) {
        std::cerr << "Trail mask with key " << key << " already exists." << std::endl;
        return;
    }

    static_cast<TextTexture*>(it->second.texture.get())->createTexture(text, fontAtlas_);
}

void TrailMapController::loadTrailMaskFromImage(std::string imageName) {

    SDL_Surface* loadedImage = loadImageFromFile(pictureFilePath_, imageName, pictureFileExtension_);
    std::string key = "Image." + imageName;
    loadImageFromSurface(key, loadedImage);
}

void TrailMapController::loadImageFromSurface(const std::string& key, SDL_Surface* surface) {
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
    
    trailMasks_[key].texture = std::make_unique<Texture>(std::move(tempTexture));
    trailMasks_[key].loadedToGPU = true;

    SDL_DestroySurface(surface);
}

void TrailMapController::loadPictureNames() {
    
    std::vector<std::string> pictureNames;

    getFileNamesInDirectory(pictureFilePath_, pictureFileExtension_, pictureNames);

    Texture tempTexture = Texture();

    for (std::string pictureName : pictureNames) {
        std::string key = "Image." + pictureName;

        trailMasks_.emplace(key, TrailMask{
            pictureName, 
            TrailMaskType::IMAGE,
            std::make_unique<Texture>(std::move(tempTexture)), 
            false, 
            std::nullopt
        });
    }
}

void TrailMapController::loadRandomPicture() {
    if(!trailMasks_.empty()) {
        
        SDL_GetCurrentTime(&timeTicks_);
        SDL_TimeToDateTime(timeTicks_, &dateTime_, true);
        
        long int randomIndex = 0;
        std::string imageName = "";
        std::string selectedKey = "";
        
        // try until a random picture passes the timetable check
        do {
            randomIndex = static_cast<long int>((size_t)rand() % trailMasks_.size());

            const auto selectedTrailMask = std::next(trailMasks_.begin(), randomIndex);
            selectedKey = selectedTrailMask->first;
            imageName = selectedTrailMask->second.name;
        } while(!checkTimeTable(imageName));
        
        //QUICK HACK: trailMasks get visually ugly, when the sensor distance is too large
        //reduce strength to 0 temporarily to allow slime behavior and colors to appear as intended
        if(appState_->slimeSettings.sensorDistance > appState_->disableAtSensorDistance) {
            //trailMaskStrengthTemp_ = appState_->universalShaderSettings.trailMaskInfluence;
            appState_->universalShaderSettings.trailMaskInfluence = 0.0f;
        } else {
            appState_->universalShaderSettings.trailMaskInfluence = trailMaskStrengthTemp_; //restore previous value
        }

        activeTrailMaskName_ = selectedKey;
        appState_->usedTrailMaskName = selectedKey;
        
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

    const auto trailMask = trailMasks_.find(activeTrailMaskName_);
    if(trailMask == trailMasks_.end() || !trailMask->second.texture) {
        std::cerr << "Failed to bind trail mask: No valid active trail mask or valid texture!\n";
        return;
    }

    glActiveTexture(GL_TEXTURE0 + textureUnit_);
    glBindTexture(GL_TEXTURE_2D, trailMask->second.texture->getID());
}

void TrailMapController::editTrailMask(const std::string& key, TrailMaskData newData) {
    if(!trailMasks_.contains(key)) {
        return;
    }

    TrailMask& trailMask = trailMasks_[key];

    trailMask.timeSlot = newData.timeSlot;

    trailMask.strength = newData.strength;
    trailMask.position = newData.position;
    trailMask.scale = newData.scale;

    if(trailMask.type == TrailMaskType::TEXT && trailMask.name != newData.name) {
        ((TextTexture*)trailMask.texture.get())->createTexture(newData.name, fontAtlas_);
    }

    trailMask.name = newData.name;
}

void TrailMapController::deleteTrailMask(const std::string& key) {

    trailMasks_.erase(key);

    //ensure activeTrailMaskName_ remains valid after deletion
    if(key == activeTrailMaskName_) {
        activeTrailMaskName_ = trailMasks_.empty() ? "" : trailMasks_.begin()->first;
        appState_->usedTrailMaskName = activeTrailMaskName_;
    }
}

void TrailMapController::onNotify(const UserEvent event) {

    switch (event.type) {
        case EventType::IMAGE_PRESET_APPLY:
        case EventType::TEXT_PRESET_APPLY: 
        {
            activeTrailMaskName_ = appState_->usedTrailMaskName;

            if(trailMasks_[activeTrailMaskName_].loadedToGPU) {
                glActiveTexture(GL_TEXTURE0 + textureUnit_);
                glBindTexture(GL_TEXTURE_2D, trailMasks_[activeTrailMaskName_].texture->getID());
            } else {
                loadTrailMaskFromImage(trailMasks_[activeTrailMaskName_].name);
            }
            appState_->universalShaderSettings.trailMaskInfluence = trailMasks_[activeTrailMaskName_].strength;
            appState_->universalShaderSettings.trailMaskPosition = trailMasks_[activeTrailMaskName_].position;
            appState_->universalShaderSettings.trailMaskScaleX = trailMasks_[activeTrailMaskName_].scale.x;
            appState_->universalShaderSettings.trailMaskScaleY = trailMasks_[activeTrailMaskName_].scale.y;

            break;
        }
        case EventType::TEXT_PRESET_CREATE:
        {
            loadTrailMaskFromText(std::get<std::string>(event.payload));
            break;
        }
        case EventType::IMAGE_PRESET_EDIT:
        case EventType::TEXT_PRESET_EDIT:
        {
            TrailMaskData newData = std::get<TrailMaskData>(event.payload);
            std::string key = (newData.type == TrailMaskType::TEXT ? "Text." : "Image.") + newData.name;

            editTrailMask(key, newData);
            break;
        }
        case EventType::TEXT_PRESET_DELETE:
        {
            TrailMaskData data = std::get<TrailMaskData>(event.payload);
            std::string key = (data.type == TrailMaskType::TEXT ? "Text." : "Image.") + data.name;
            deleteTrailMask(key);
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
