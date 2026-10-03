#ifndef TRAIL_MASK_H
#define TRAIL_MASK_H

#include <chrono>
#include <memory>
#include <optional>
#include <string>


#include "../application_state.h"
#include "../graphics/font_atlas.h"
#include "../graphics/texture.h"
#include "../utility/time_handling.h"  // for TimeSlot
#include "../utility/vector_math.h"    // for phys::Vec2

/**
 * @brief Properties of a trail mask.
 * 
 * Used only for the constructor of TrailMask, so when defining only some properties, the user isn't constrained by the order of the constructor parameters.
 */
struct TrailMaskProperties {
        std::string text{""};
        std::optional<TimeSlot> timeSlot = std::nullopt;

        float strength{1.0f};
        phys::Vec2<float> position{0.0f, 0.0f};
        phys::Vec2<float> scale{1.0f, 1.0f};    //external user defined scale

        bool isInverted{false};
};

enum class TrailMaskType {
    IMAGE,
    TEXT
};

class TrailMask {
public:
        std::string name;
        TrailMaskType type;

        std::string text{""};
        std::unique_ptr<Texture> texture{nullptr};

        std::optional<TimeSlot> timeSlot = std::nullopt;

        float strength{1.0f};
        phys::Vec2<float> position{0.0f, 0.0f};
        phys::Vec2<float> scale{1.0f, 1.0f};    //external user defined scale
        bool isInverted{false};

        TrailMask() = delete;   //only allow construction with at least a name and type

        TrailMask(std::string nameIn, TrailMaskType typeIn, TrailMaskProperties propertiesIn = TrailMaskProperties{});

        TrailMask(const TrailMask&);
        TrailMask& operator=(const TrailMask&);
        TrailMask(TrailMask&&);
        TrailMask& operator=(TrailMask&&);

        void createTextureFromImage(std::string& pictureFilePath, std::string& imageName, std::string& pictureFileExtension, ApplicationState* appState);
        void createTextureFromText(std::string& text, FontAtlas& fontAtlas, ApplicationState* appState);

        /**
         * @brief Generates a key for use in maps based on its type and name.
         * 
         * @return key
         * 
         * @note Uniqueness of the key must be ensured by the caller.
         */
        std::string makeKey() { return makeKey(type, name); };

        /**
         * @brief Generates a key for use in maps based on external parameters.
         * 
         * For use before an instance of TrailMask is created, e.g. when using emplace() with a map.
         * 
         * @param type The type of the trail mask (IMAGE or TEXT).
         * @param name The name of the trail mask.
         * @return key
         * 
         * @note This function does not guarantee uniqueness of the generated key.
         */
        static std::string makeKey(const TrailMaskType type, const std::string& name);

        phys::Vec2<float> getAspectRatioCorrection() const { return aspectRatioCorrection_; }

private:
        phys::Vec2<float> aspectRatioCorrection_{1.0f, 1.0f};    // internally used to correct the aspect ratio of the trail mask texture, because the textures are warped by OpenGL to fill the whole canvas
};

#endif // TRAIL_MASK_H