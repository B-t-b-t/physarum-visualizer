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

float TrailMapController::globalStrength_ = 1.0f;
phys::Vec2<float> TrailMapController::globalPosition_{0.0f, 0.0f};
phys::Vec2<float> TrailMapController::globalScale_{1.0f, 1.0f};

TrailMapController::TrailMapController(std::string pictureFilePath, std::string pictureFileExtension, GLuint textureUnit, ApplicationState* appState)
 : pictureFilePath_(pictureFilePath), 
   pictureFileExtension_(pictureFileExtension),
   textureUnit_(textureUnit),
   appState_(appState),
   fontAtlas_{FontAtlas("Roboto-Medium")}
{
    loadEntriesFromToml();     //load from saveFile first
    loadEntriesFromDirectory();   //load remaining images from the directory (duplicates with TOML entries get ignored)
    createTrailMaskTextures();

    activeTrailMaskName_ = trailMasks_.empty() ? "" : trailMasks_.begin()->first;

    appState_->trailMasks = &trailMasks_;
    appState_->usedTrailMaskName = activeTrailMaskName_;
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
    timeTicks_ = other.timeTicks_;
    dateTime_ = other.dateTime_;
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
        timeTicks_ = other.timeTicks_;
        dateTime_ = other.dateTime_;
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

std::string makeKey(TrailMaskType type, const std::string& name) {
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

std::string TrailMask::makeKey() {
    return ::makeKey(type, name);
}

std::string TrailMaskData::makeKey() {
    return ::makeKey(type, name);
}

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

bool TrailMapController::loadEntriesFromToml() {
    toml::value timeTable;
    //parse
    try {
        timeTable = toml::parse("./res/pictures/timeTable.toml");
    } catch(const toml::exception& err) {
        std::cerr << "Failed to parse timeTable.toml: " << err.what() << std::endl;
        return false;
    }

    const toml::array textEntries = toml::find_or<toml::array>(timeTable, "TEXT", toml::array{});

    //get all text entries
    for(const auto& entry : textEntries) {
        if(!entry.contains("text")) {   //discard entries without a "text" field
            continue;
        }
        std::string name = toml::find<std::string>(entry, "name");
        std::string text = toml::find<std::string>(entry, "text");
        std::optional<TimeSlot> timeSlot = std::nullopt;

        if(entry.contains("start") && entry.contains("end")) {
            std::chrono::system_clock::time_point start = toml::find<std::chrono::system_clock::time_point>(entry, "start");
            std::chrono::system_clock::time_point end = toml::find<std::chrono::system_clock::time_point>(entry, "end");
            timeSlot = TimeSlot{start, end};
        }

        float strength = toml::find_or<float>(entry, "strength", 1.0f);
        float positionX = toml::find_or<float>(entry, "position", 0, 0.0f);
        float positionY = toml::find_or<float>(entry, "position", 1, 0.0f);
        float scaleX = toml::find_or<float>(entry, "scale", 0, 1.0f);
        float scaleY = toml::find_or<float>(entry, "scale", 1, 1.0f);

        std::string key = makeKey(TrailMaskType::TEXT, name);

        //create entry
        trailMasks_.try_emplace(key, TrailMask{
            text, 
            TrailMaskType::TEXT,
            std::make_unique<TextTexture>(appState_->universalShaderSettings.textureWidth, appState_->universalShaderSettings.textureHeight, appState_),
            timeSlot,
            strength,
            phys::Vec2{positionX, positionY},
            phys::Vec2{scaleX, scaleY}
        });
    }

    const toml::array imageEntries = toml::find_or<toml::array>(timeTable, "IMAGE", toml::array{});

    //get all image entries
    for(const auto& entry : imageEntries) {
        std::string name = toml::find<std::string>(entry, "name");
        std::optional<TimeSlot> timeSlot = std::nullopt;

        if(entry.contains("start") && entry.contains("end")) {
            std::chrono::system_clock::time_point start = toml::find<std::chrono::system_clock::time_point>(entry, "start");
            std::chrono::system_clock::time_point end = toml::find<std::chrono::system_clock::time_point>(entry, "end");
            timeSlot = TimeSlot{start, end};
        }

        float strength = toml::find_or<float>(entry, "strength", 1.0f);
        float positionX = toml::find_or<float>(entry, "position", 0, 0.0f);
        float positionY = toml::find_or<float>(entry, "position", 1, 0.0f);
        float scaleX = toml::find_or<float>(entry, "scale", 0, 1.0f);
        float scaleY = toml::find_or<float>(entry, "scale", 1, 1.0f);

        std::string key = makeKey(TrailMaskType::IMAGE, name);

        //create entry
        trailMasks_.try_emplace(key, TrailMask{
            name, 
            TrailMaskType::IMAGE,
            nullptr,
            timeSlot,
            strength,
            phys::Vec2{positionX, positionY},
            phys::Vec2{scaleX, scaleY}
        });
    }

    return true;
}

void TrailMapController::createTrailMaskTextures() {
    //load textures
    for(auto& [key, trailMask] : trailMasks_) {
        switch(trailMask.type) {
            case TrailMaskType::IMAGE: {
                loadTrailMaskFromImage(trailMask.name);
                int texWidth = trailMask.texture->getWidth();
                int texHeight = trailMask.texture->getHeight();
                float sizeRatio = texWidth / (float) texHeight;
                float canvasRatio = appState_->universalShaderSettings.textureWidth / (float) appState_->universalShaderSettings.textureHeight;

                if(sizeRatio > 1.0f) {     //wider than tall
                    if(sizeRatio > canvasRatio) {
                        trailMask.aspectRatioCorrection.y *= canvasRatio / sizeRatio;
                    } else {
                        trailMask.aspectRatioCorrection.x *= sizeRatio / canvasRatio;
                    }
                } else if(sizeRatio < 1.0f) {   // taller than wide
                    trailMask.aspectRatioCorrection.x *= sizeRatio / canvasRatio;
                }
            }                
                break;
            case TrailMaskType::TEXT:
                static_cast<TextTexture*>(trailMask.texture.get())->createTexture(trailMask.name, fontAtlas_);
                break;
        }
    }
}

bool TrailMapController::saveToToml() {
    toml::value newTimeTable{toml::table{}};
    toml::array textEntries;
    toml::array imageEntries;

    constexpr float epsilon = 1e-6f;

    for(const auto& [key, trailMask] : trailMasks_) {
        //images without any additional data are discovered from the image directory and don't need
        //to be saved. Text masks must always be saved.
        if(trailMask.type == TrailMaskType::IMAGE && !trailMask.timeSlot) {
            continue;
        }

        //values in toml file are saved in reverse order from this code
        toml::value entry{toml::table{}};
        
        if(std::abs(trailMask.scale.x - 1.0f) > epsilon 
        || std::abs(trailMask.scale.y - 1.0f) > epsilon) {
            entry["scale"] = toml::array{
                trailMask.scale.x,
                trailMask.scale.y
            };
        }
        
        if(std::abs(trailMask.position.x) > epsilon 
        || std::abs(trailMask.position.y) > epsilon) {
            entry["position"] = toml::array{
                trailMask.position.x,
                trailMask.position.y
            };
        }
        
        if(std::abs(trailMask.strength - 1.0f) > epsilon) { entry["strength"] = trailMask.strength; }

        if(trailMask.timeSlot) {
            entry["end"] = toml::offset_datetime(trailMask.timeSlot->end);
            entry["start"] = toml::offset_datetime(trailMask.timeSlot->start);
        }

        if(trailMask.type == TrailMaskType::TEXT) { entry["text"] = trailMask.name; }

        entry["name"] = trailMask.name;

        if(trailMask.type == TrailMaskType::TEXT) {
            textEntries.push_back(std::move(entry));
        } else {
            imageEntries.push_back(std::move(entry));
        }
    }

    if(!textEntries.empty()) {
        newTimeTable["TEXT"] = std::move(textEntries);
    }

    if(!imageEntries.empty()) {
        newTimeTable["IMAGE"] = std::move(imageEntries);
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

    return true;
}

void TrailMapController::loadTrailMaskFromImage(std::string imageName) {

    SDL_Surface* loadedImage = loadImageFromFile(pictureFilePath_, imageName, pictureFileExtension_);

    TextureProperties properties;
    properties.width = loadedImage->w;
    properties.height = loadedImage->h;
    properties.wrapX = TextureWrap::CLAMP_TO_BORDER;
    properties.wrapY = TextureWrap::CLAMP_TO_BORDER;
    properties.minFilter = TextureMinFilter::LINEAR;
    properties.magFilter = TextureMagFilter::LINEAR;
    properties.generateMipmaps = false;

    Texture tempTexture(properties, loadedImage->pixels, TextureDataFormat::RGBA, TextureDataType::UBYTE, loadedImage->pitch);
    
    trailMasks_[makeKey(TrailMaskType::IMAGE, imageName)].texture = std::make_unique<Texture>(std::move(tempTexture));

    SDL_DestroySurface(loadedImage);
}

void TrailMapController::createTrailMaskFromText(const std::string& text) {
    std::string key = makeKey(TrailMaskType::TEXT, text);

    //not inserted if the key already exists
    const auto& [iter, inserted] = trailMasks_.try_emplace(key, 
        TrailMask{
        text, 
        TrailMaskType::TEXT,
        std::make_unique<TextTexture>(appState_->universalShaderSettings.textureWidth, appState_->universalShaderSettings.textureHeight, appState_)
    });

    if(inserted) {
        auto& trailMask = trailMasks_.at(key);
        static_cast<TextTexture*>(trailMask.texture.get())->createTexture(trailMask.name, fontAtlas_);
    }
}

void TrailMapController::loadEntriesFromDirectory() {
    
    std::vector<std::string> pictureNames;

    getFileNamesInDirectory(pictureFilePath_, pictureFileExtension_, pictureNames);

    for (std::string pictureName : pictureNames) {
        std::string key = makeKey(TrailMaskType::IMAGE, pictureName);

        //not inserted if the key already exists
        trailMasks_.try_emplace(key, 
            TrailMask{
            pictureName, 
            TrailMaskType::IMAGE,
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
    trailMask.isInverted = newData.isInverted;

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

            if(trailMasks_[activeTrailMaskName_].texture != nullptr) {
                glActiveTexture(GL_TEXTURE0 + textureUnit_);
                glBindTexture(GL_TEXTURE_2D, trailMasks_[activeTrailMaskName_].texture->getID());
            } else {
                loadTrailMaskFromImage(trailMasks_[activeTrailMaskName_].name);
            }
            appState_->universalShaderSettings.trailMaskInfluence = trailMasks_[activeTrailMaskName_].strength + globalStrength_;
            appState_->universalShaderSettings.trailMaskPosition = trailMasks_[activeTrailMaskName_].position + globalPosition_;
            appState_->universalShaderSettings.trailMaskScaleX = trailMasks_[activeTrailMaskName_].scale.x * trailMasks_[activeTrailMaskName_].aspectRatioCorrection.x * globalScale_.x;
            appState_->universalShaderSettings.trailMaskScaleY = trailMasks_[activeTrailMaskName_].scale.y * trailMasks_[activeTrailMaskName_].aspectRatioCorrection.y * globalScale_.y;
            appState_->universalShaderSettings.trailMaskIsInverted = trailMasks_[activeTrailMaskName_].isInverted;

            break;
        }
        case EventType::TEXT_PRESET_CREATE:
        {
            createTrailMaskFromText(std::get<std::string>(event.payload));
            break;
        }
        case EventType::IMAGE_PRESET_EDIT:
        case EventType::TEXT_PRESET_EDIT:
        {
            TrailMaskData newData = std::get<TrailMaskData>(event.payload);
            editTrailMask(newData.makeKey(), newData);
            break;
        }
        case EventType::TEXT_PRESET_DELETE:
        {
            TrailMaskData data = std::get<TrailMaskData>(event.payload);
            deleteTrailMask(data.makeKey());
            break;
        }
        default:
            break;
    }
}
