#ifndef TRAIL_MAP_CONTROLLER_H
#define TRAIL_MAP_CONTROLLER_H

#include <map>
#include <string>

#include <GL/glew.h>

#include "../application_state.h"
#include "../graphics/font_atlas.h"
#include "../graphics/texture.h"
#include "../graphics/text_texture.h"
#include "../utility/event.h"
#include "../utility/observer.h"
#include "../utility/vector_math.h"
#include "trail_mask.h"

class TrailMapController : public Observer {
public:

    TrailMapController() = default;
    TrailMapController(std::string pictureFilePath, std::string pictureFileExtension, GLuint textureUnit, ApplicationState* appState);
    TrailMapController(const TrailMapController&) = delete;
    TrailMapController& operator=(const TrailMapController&) = delete;
    TrailMapController(TrailMapController&&);
    TrailMapController& operator=(TrailMapController&&);
    ~TrailMapController();
    
    void bindToTextureUnit(GLuint textureUnit);
    
	void autoSwitchTrailMasks(uint64_t timeInSeconds);
    void loadRandomTrailMask();
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

    std::string pictureFilePath_ = "./res/pictures/";
    std::string pictureFileExtension_ = ".png";
    GLuint textureUnit_;	//Default Texture Unit for Trail Mask Texture
    ApplicationState* appState_ = nullptr;

    FontAtlas fontAtlas_;

};

#endif // TRAIL_MAP_CONTROLLER_H