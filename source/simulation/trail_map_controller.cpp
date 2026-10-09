#include "trail_map_controller.h"

#include <cmath>                // for std::abs
#include <filesystem>
#include <fstream>
#include <iostream>
#include <iterator>             // for next

#include <toml.hpp>

#include "../application_state.h"
#include "../ui/windows/preset_window.h"
#include "../utility/event.h"
#include "../utility/fileHandling.h"
#include "../utility/filepaths.h"

float TrailMapController::globalStrength_ = 1.0f;
phys::Vec2<float> TrailMapController::globalPosition_{0.0f, 0.0f};
phys::Vec2<float> TrailMapController::globalScale_{1.0f, 1.0f};

TrailMapController::TrailMapController(FilePaths* paths, GLuint textureUnit, ApplicationState* appState)
 : paths_(paths),
   textureUnit_(textureUnit),
   appState_(appState),
   fontAtlas_{FontAtlas(paths->fontFileDir / paths->fontFile, paths->fontAtlasTestOutputImage)}
{
    loadEntriesFromToml();     //load from saveFile first
    loadEntriesFromDirectory();   //load remaining images from the directory (duplicates with TOML entries get ignored)
    createAllTrailMaskTextures();

    trailMasks_.try_emplace("Empty._None_", TrailMask{"_None_", TrailMaskType::EMPTY});

    activeTrailMaskKey_ = trailMasks_.find("Empty._None_")->first;

    appState_->trailMasks = &trailMasks_;
    appState_->usedTrailMaskName = activeTrailMaskKey_;
}

TrailMapController::TrailMapController(TrailMapController&& other) {
    paths_ = other.paths_;
    textureUnit_ = other.textureUnit_;
    appState_ = other.appState_;
    fontAtlas_ = std::move(other.fontAtlas_);
    trailMasks_ = std::move(other.trailMasks_);
    activeTrailMaskKey_ = other.activeTrailMaskKey_;

    //inform appState after move just in case
    if(appState_) {
        appState_->trailMasks = &trailMasks_;
        appState_->usedTrailMaskName = activeTrailMaskKey_;
    }

    other.textureUnit_ = 0;
    other.appState_ = nullptr;
}

TrailMapController& TrailMapController::operator=(TrailMapController&& other) {
    if(this != &other) {
        paths_ = std::move(other.paths_);
        textureUnit_ = other.textureUnit_;
        appState_ = other.appState_;
        fontAtlas_ = std::move(other.fontAtlas_);
        trailMasks_ = std::move(other.trailMasks_);
        activeTrailMaskKey_ = other.activeTrailMaskKey_;

        //inform appState after move just in case
        if(appState_) {
            appState_->trailMasks = &trailMasks_;
            appState_->usedTrailMaskName = activeTrailMaskKey_;
        }

        other.textureUnit_ = 0;
        other.appState_ = nullptr;
    }
    return *this;
}

TrailMapController::~TrailMapController() {
    saveToToml();
}

bool TrailMapController::loadEntriesFromToml() {
    toml::value timeTable;
    //parse
    try {
        timeTable = toml::parse(paths_->timeTableFilePath);
    } catch(const toml::exception& err) {
        std::cerr << "Failed to parse " << paths_->timeTableFilePath << " : " << err.what() << std::endl;
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

        int animationType = toml::find_or<int>(entry, "animationType", 0);
        double positionVelocityX = toml::find_or<double>(entry, "positionVelocity", 0, 0.0f);
        double positionVelocityY = toml::find_or<double>(entry, "positionVelocity", 1, 0.0f);
        float positionStartX = toml::find_or<float>(entry, "positionStart", 0, 0.0f);
        float positionStartY = toml::find_or<float>(entry, "positionStart", 1, 0.0f);
        float positionEndX = toml::find_or<float>(entry, "positionEnd", 0, 0.0f);
        float positionEndY = toml::find_or<float>(entry, "positionEnd", 1, 0.0f);
        double scaleVelocity = toml::find_or<double>(entry, "scaleVelocity", 0.0f);
        float scaleStart = toml::find_or<float>(entry, "scaleStart", 1.0f);
        float scaleEnd = toml::find_or<float>(entry, "scaleEnd", 1.0f);

        //create entry
        std::string key = TrailMask::makeKey(TrailMaskType::TEXT, name);

        TrailMaskProperties properties{
            text,
            timeSlot,
            strength,
            phys::Vec2{positionX, positionY},
            phys::Vec2{scaleX, scaleY}
        };

        switch (animationType) {
            case 0:
                properties.animation = std::nullopt; // None
                break;
            case 1:
                properties.animation = TrailMaskAnimation{phys::Vec2{positionVelocityX, positionVelocityY}, phys::Vec2{positionStartX, positionStartY}, phys::Vec2{positionEndX, positionEndY}};
                break;
            case 2:
                properties.animation = TrailMaskAnimation{scaleVelocity, scaleStart, scaleEnd};
                break;
        }

        trailMasks_.try_emplace(key, TrailMask{ name, TrailMaskType::TEXT, properties });
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

        int animationType = toml::find_or<int>(entry, "animationType", 0);
        double positionVelocityX = toml::find_or<double>(entry, "positionVelocity", 0, 0.0f);
        double positionVelocityY = toml::find_or<double>(entry, "positionVelocity", 1, 0.0f);
        float positionStartX = toml::find_or<float>(entry, "positionStart", 0, 0.0f);
        float positionStartY = toml::find_or<float>(entry, "positionStart", 1, 0.0f);
        float positionEndX = toml::find_or<float>(entry, "positionEnd", 0, 0.0f);
        float positionEndY = toml::find_or<float>(entry, "positionEnd", 1, 0.0f);
        double scaleVelocity = toml::find_or<double>(entry, "scaleVelocity", 0.0f);
        float scaleStart = toml::find_or<float>(entry, "scaleStart", 1.0f);
        float scaleEnd = toml::find_or<float>(entry, "scaleEnd", 1.0f);

        //create entry
        std::string key = TrailMask::makeKey(TrailMaskType::IMAGE, name);

        TrailMaskProperties properties{
            "",
            timeSlot,
            strength,
            phys::Vec2{positionX, positionY},
            phys::Vec2{scaleX, scaleY}
        };

        switch (animationType) {
            case 0:
                properties.animation = std::nullopt; // None
                break;
            case 1:
                properties.animation = TrailMaskAnimation{phys::Vec2{positionVelocityX, positionVelocityY}, phys::Vec2{positionStartX, positionStartY}, phys::Vec2{positionEndX, positionEndY}};
                break;
            case 2:
                properties.animation = TrailMaskAnimation{scaleVelocity, scaleStart, scaleEnd};
                break;
        }

        trailMasks_.try_emplace(key, TrailMask{ name, TrailMaskType::IMAGE, properties});
    }

    return true;
}

void TrailMapController::createAllTrailMaskTextures() {
    //load textures
    for(auto& [key, trailMask] : trailMasks_) {
        switch(trailMask.type) {
            case TrailMaskType::IMAGE: {
                std::filesystem::path filePath{""};
                filePath += paths_->pictureDir;
                filePath /= trailMask.name;
                filePath += paths_->pictureFileExtension;
                trailMask.createTextureFromImage(filePath, appState_);
            }                
                break;
            case TrailMaskType::TEXT:
                trailMask.createTextureFromText(fontAtlas_, paths_, appState_);
                break;
            default:
                break;
        }
    }
}

bool TrailMapController::saveToToml() {
    toml::value newTimeTable{toml::table{}};
    toml::array textEntries;
    toml::array imageEntries;

    constexpr float EPSILON = 1e-6f;

    for(const auto& [key, trailMask] : trailMasks_) {
        
        if(trailMask.type == TrailMaskType::EMPTY) {
            continue;
        }

        //values in toml file are saved in reverse order from this code
        toml::value entry{toml::table{}};
        
        if(trailMask.animation) {
            if(trailMask.animation.value().hasTranslation()) {
                entry["positionEnd"] = toml::array{
                    trailMask.animation.value().positionEnd().x,
                    trailMask.animation.value().positionEnd().y
                };
                entry["positionStart"] = toml::array{
                    trailMask.animation.value().positionStart().x,
                    trailMask.animation.value().positionStart().y
                };
                entry["positionVelocity"] = toml::array{
                    trailMask.animation.value().positionVelocity().x,
                    trailMask.animation.value().positionVelocity().y
                };
                entry["animationType"] = 1;
            } else if(trailMask.animation.value().hasScaling()) {
                entry["scaleEnd"] = trailMask.animation.value().scaleEnd();
                entry["scaleStart"] = trailMask.animation.value().scaleStart();
                entry["scaleVelocity"] = trailMask.animation.value().scaleVelocity();
                entry["animationType"] = 2;
            }
        } else {
            entry["animationType"] = 0;
        }

        if(std::abs(trailMask.scale.x - 1.0f) > EPSILON 
        || std::abs(trailMask.scale.y - 1.0f) > EPSILON) {
            entry["scale"] = toml::array{
                trailMask.scale.x,
                trailMask.scale.y
            };
        }
        
        if(std::abs(trailMask.position.x) > EPSILON 
        || std::abs(trailMask.position.y) > EPSILON) {
            entry["position"] = toml::array{
                trailMask.position.x,
                trailMask.position.y
            };
        }
        
        if(std::abs(trailMask.strength - 1.0f) > EPSILON) { entry["strength"] = trailMask.strength; }

        if(trailMask.timeSlot) {
            entry["end"] = toml::offset_datetime(trailMask.timeSlot->end);
            entry["start"] = toml::offset_datetime(trailMask.timeSlot->start);
        }

        if(trailMask.type == TrailMaskType::TEXT) { entry["text"] = trailMask.text; }

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

    std::ofstream outputFile{paths_->timeTableFilePath, std::ios::trunc};
    if(!outputFile.is_open()) {
        std::cerr << "Failed to open " << paths_->timeTableFilePath << " for writing" << std::endl;
        return false;
    }

    outputFile << toml::format(newTimeTable);
    outputFile.flush();

    if(!outputFile.good()) {
        std::cerr << "Failed to write to " << paths_->timeTableFilePath << std::endl;
        return false;
    }

    return true;
}

void TrailMapController::loadEntriesFromDirectory() {
    
    std::vector<std::string> pictureNames;

    getFileNamesInDirectory(paths_->pictureDir, paths_->pictureFileExtension, pictureNames);

    for (std::string pictureName : pictureNames) {
        std::string key = TrailMask::makeKey(TrailMaskType::IMAGE, pictureName);

        //not inserted if the key already exists
        trailMasks_.try_emplace(key, 
            TrailMask{
            pictureName, 
            TrailMaskType::IMAGE,
        });
    }
}

void TrailMapController::bindToTextureUnit(GLuint textureUnit) { 
    if(trailMasks_.at(activeTrailMaskKey_).type == TrailMaskType::EMPTY) {
        return;
    }

    textureUnit_ = textureUnit;

    const auto trailMask = trailMasks_.find(activeTrailMaskKey_);
    if(trailMask == trailMasks_.end() || !trailMask->second.texture) {
        std::cerr << "Failed to bind trail mask: No valid active trail mask or valid texture!\n";
        return;
    }

    glActiveTexture(GL_TEXTURE0 + textureUnit_);
    glBindTexture(GL_TEXTURE_2D, trailMask->second.texture->getID());
}

void TrailMapController::moveTrailMask() {
    TrailMask& trailMask = trailMasks_.at(activeTrailMaskKey_);
    //move until destination is reached
    if(trailMask.animation.has_value() && !trailMask.hasReachedDest(appState_)) {
        appState_->universalShaderSettings.trailMaskPosition.x += (double)trailMask.animation.value().positionVelocity().x;
        appState_->universalShaderSettings.trailMaskPosition.y += (double)trailMask.animation.value().positionVelocity().y;
        appState_->universalShaderSettings.trailMaskScale.x += trailMask.animation.value().scaleVelocity() * appState_->universalShaderSettings.trailMaskScale.x;
        appState_->universalShaderSettings.trailMaskScale.y += trailMask.animation.value().scaleVelocity() * appState_->universalShaderSettings.trailMaskScale.y;
    }
}

void TrailMapController::editTrailMask(const std::string& key, TrailMask newData) {
    if(!trailMasks_.contains(key)) {
        return;
    }

    TrailMask& trailMask = trailMasks_.at(key);

    trailMask.timeSlot = newData.timeSlot;

    trailMask.strength = newData.strength;
    trailMask.position = newData.position;
    trailMask.scale = newData.scale;
    trailMask.animation = newData.animation;
    trailMask.isInverted = newData.isInverted;

    if(trailMask.type == TrailMaskType::TEXT && trailMask.text != newData.text) {
        trailMask.text = newData.text;
        trailMask.createTextureFromText(fontAtlas_, paths_, appState_);
    }

    trailMask.name = newData.name;
}

void TrailMapController::deleteTrailMask(const std::string& key) {

    trailMasks_.erase(key);

    //ensure activeTrailMaskName_ remains valid after deletion
    if(key == activeTrailMaskKey_) {
        activeTrailMaskKey_ = trailMasks_.empty() ? "" : trailMasks_.begin()->first;
        appState_->usedTrailMaskName = activeTrailMaskKey_;
    }
}

void TrailMapController::onNotify(const UserEvent event) {

    switch (event.type) {
        case EventType::IMAGE_PRESET_APPLY:
        case EventType::TEXT_PRESET_APPLY: 
        {
            activeTrailMaskKey_ = appState_->usedTrailMaskName;
            TrailMask& trailMask = trailMasks_.at(activeTrailMaskKey_);

            if(trailMask.texture != nullptr) {
                glActiveTexture(GL_TEXTURE0 + textureUnit_);
                glBindTexture(GL_TEXTURE_2D, trailMask.texture->getID());
            }
            appState_->universalShaderSettings.trailMaskInfluence = trailMask.strength + globalStrength_;
            appState_->universalShaderSettings.trailMaskPosition = toStd140(trailMask.position + globalPosition_);
            appState_->universalShaderSettings.trailMaskScale.x = trailMask.scale.x * trailMask.getAspectRatioCorrection().x * globalScale_.x;
            appState_->universalShaderSettings.trailMaskScale.y = trailMask.scale.y * trailMask.getAspectRatioCorrection().y * globalScale_.y;
            appState_->universalShaderSettings.trailMaskIsInverted = trailMask.isInverted;

            if(trailMask.animation) {
                appState_->universalShaderSettings.trailMaskPosition.x += trailMask.animation.value().positionStart().x;
                appState_->universalShaderSettings.trailMaskPosition.y += trailMask.animation.value().positionStart().y;
                appState_->universalShaderSettings.trailMaskScale.x *= trailMask.animation.value().scaleStart();
                appState_->universalShaderSettings.trailMaskScale.y *= trailMask.animation.value().scaleStart();
            }

            break;
        }
        case EventType::TEXT_PRESET_CREATE:
        {   TrailMask data = std::get<TrailMask>(event.payload);
            std::string key = TrailMask::makeKey(TrailMaskType::TEXT, data.name);

            //not inserted if the key already exists
            const auto& [iter, inserted] = trailMasks_.try_emplace(key, data);
            if(inserted) {
                iter->second.createTextureFromText(fontAtlas_, paths_, appState_);
            }
            break;
        }
        case EventType::IMAGE_PRESET_EDIT:
        case EventType::TEXT_PRESET_EDIT:
        {
            TrailMask newData = std::get<TrailMask>(event.payload);
            editTrailMask(newData.makeKey(), newData);
            break;
        }
        case EventType::TEXT_PRESET_DELETE:
        {
            TrailMask data = std::get<TrailMask>(event.payload);
            deleteTrailMask(data.makeKey());
            break;
        }
        default:
            break;
    }
}
