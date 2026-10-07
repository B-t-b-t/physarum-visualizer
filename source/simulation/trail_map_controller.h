#ifndef TRAIL_MAP_CONTROLLER_H
#define TRAIL_MAP_CONTROLLER_H

#include <filesystem>
#include <map>
#include <string>

#include <GL/glew.h>

#include "../application_state.h"
#include "../graphics/font_atlas.h"
#include "../graphics/texture.h"
#include "../graphics/text_texture.h"
#include "../utility/event.h"
#include "../utility/filepaths.h"
#include "../utility/observer.h"
#include "../utility/vector_math.h"
#include "trail_mask.h"

class TrailMapController : public Observer {
public:

    TrailMapController() = default;
    TrailMapController(FilePaths* paths, GLuint textureUnit, ApplicationState* appState);
    TrailMapController(const TrailMapController&) = delete;
    TrailMapController& operator=(const TrailMapController&) = delete;
    TrailMapController(TrailMapController&&);
    TrailMapController& operator=(TrailMapController&&);
    ~TrailMapController();
    
    void bindToTextureUnit(GLuint textureUnit);
    
    void editTrailMask(const std::string& key, TrailMask newData);
    void deleteTrailMask(const std::string& key);
    void onNotify(const UserEvent event) override;
    
    static float globalStrength_;
    static phys::Vec2<float> globalPosition_;
    static phys::Vec2<float> globalScale_;

    private:
    
    bool loadEntriesFromToml();
    void loadEntriesFromDirectory();
    bool saveToToml();
    void createAllTrailMaskTextures();

    std::map<std::string, TrailMask> trailMasks_;
    std::string activeTrailMaskKey_;

    FilePaths* paths_;   // whole struct necessary, because multiple methods need access

    GLuint textureUnit_;	//Default Texture Unit for Trail Mask Texture
    ApplicationState* appState_ = nullptr;

    FontAtlas fontAtlas_;

};

#endif // TRAIL_MAP_CONTROLLER_H