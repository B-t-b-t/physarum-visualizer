#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>

#include <GL/glew.h>
#include <SDL3/SDL.h>

#include <chrono>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <memory>
#include <random>
#include <stdexcept>
#include <string>

#include "application_state.h"
#include "simulation/scene.h"
#include "simulation/scene_controller.h"
#include "simulation/trail_map_controller.h"
#include "utility/event.h"
#include "utility/filepaths.h"
#include "utility/parameter_parser.h"

// AI generated test cases, handle with care!
// TODO: refactor trail mask code, to make it more easily testable

namespace {

class StateFixture {
public:
    StateFixture() {
        Parameters params;
        state = ApplicationState::getInstance(params);
        oldTrailMasks = state->trailMasks;
        oldBehaviorPresets = state->behaviorPresets;
        oldColorPresets = state->colorPresets;
        oldScenes = state->scenes;
        oldMaskName = state->usedTrailMaskName;
        oldSceneName = state->usedSceneName;
        oldSettings = state->universalShaderSettings;
        oldSlimeSettings = state->slimeSettings;
        oldAutoSwitching = state->autoSceneSwitching;
        oldInterval = state->sceneSwitchingIntervall;
        oldBeatVolumeSwitch = state->beatVolumeSwitch;
    }

    ~StateFixture() {
        state->trailMasks = oldTrailMasks;
        state->behaviorPresets = oldBehaviorPresets;
        state->colorPresets = oldColorPresets;
        state->scenes = oldScenes;
        state->usedTrailMaskName = oldMaskName;
        state->usedSceneName = oldSceneName;
        state->universalShaderSettings = oldSettings;
        state->slimeSettings = oldSlimeSettings;
        state->autoSceneSwitching = oldAutoSwitching;
        state->sceneSwitchingIntervall = oldInterval;
        state->beatVolumeSwitch = oldBeatVolumeSwitch;
    }

    ApplicationState* state;

private:
    std::map<std::string, TrailMask>* oldTrailMasks;
    std::map<std::string, BehaviorPreset>* oldBehaviorPresets;
    std::map<std::string, ColorPreset>* oldColorPresets;
    std::map<std::string, Scene>* oldScenes;
    std::string oldMaskName;
    std::string oldSceneName;
    UniversalShaderSettings oldSettings;
    SlimeSettings oldSlimeSettings;
    bool oldAutoSwitching;
    int oldInterval;
    int oldBeatVolumeSwitch;
};

class TemporaryDirectory {
public:
    TemporaryDirectory() {
        std::random_device random;
        for(int attempt = 0; attempt < 10; ++attempt) {
            path = std::filesystem::temp_directory_path() /
                   ("physarum-scene-test-" + std::to_string(random()));
            if(std::filesystem::create_directory(path)) {
                return;
            }
        }
        throw std::runtime_error("Unable to create a temporary scene test directory");
    }

    ~TemporaryDirectory() {
        std::error_code error;
        for(const char* name : {"scenes.toml", "timetable.toml", "fontAtlas.png"}) {
            std::filesystem::remove(path / name, error);
            if(error) {
                std::cerr << "Failed to remove test file: " << error.message() << '\n';
            }
        }
        std::filesystem::remove(path, error);
        if(error) {
            std::cerr << "Failed to remove test directory: " << error.message() << '\n';
        }
    }

    std::filesystem::path path;
};

class MaskEventRecorder : public Observer {
public:
    void onNotify(const UserEvent event) override {
        if(event.type == EventType::IMAGE_PRESET_APPLY) {
            ++count;
            key = std::get<std::string>(event.payload);
        }
    }

    int count = 0;
    std::string key;
};

} // namespace

TEST_CASE("Scene selects None for an out-of-slot mask", "[scene][trail-mask]") {
    StateFixture fixture;
    auto* state = fixture.state;
    std::map<std::string, TrailMask> masks;
    masks.try_emplace(kEmptyTrailMaskKey, "_None_", TrailMaskType::EMPTY);
    masks.try_emplace("Image.previous", "previous", TrailMaskType::IMAGE);
    state->trailMasks = &masks;
    state->usedTrailMaskName = "Image.previous";

    const auto now = std::chrono::system_clock::now();
    TrailMaskProperties properties;
    Scene scene{"test"};

    SECTION("expired image") {
        properties.timeSlot = TimeSlot{now - std::chrono::hours{48}, now - std::chrono::hours{24}};
        masks.try_emplace("Image.candidate", "candidate", TrailMaskType::IMAGE, properties);
        scene.setAssociatedImages(masks, {"Image.candidate"});
        REQUIRE(scene.applyRandomPresets(state));
        REQUIRE(state->usedTrailMaskName == kEmptyTrailMaskKey);
    }

    SECTION("future text") {
        properties.timeSlot = TimeSlot{now + std::chrono::hours{24}, now + std::chrono::hours{48}};
        masks.try_emplace("Text.candidate", "candidate", TrailMaskType::TEXT, properties);
        scene.setAssociatedTexts(masks, {"Text.candidate"});
        REQUIRE(scene.applyRandomPresets(state));
        REQUIRE(state->usedTrailMaskName == kEmptyTrailMaskKey);
    }

    SECTION("active time slot") {
        properties.timeSlot = TimeSlot{now - std::chrono::hours{1}, now + std::chrono::hours{1}};
        masks.try_emplace("Image.candidate", "candidate", TrailMaskType::IMAGE, properties);
        scene.setAssociatedImages(masks, {"Image.candidate"});
        REQUIRE(scene.applyRandomPresets(state));
        REQUIRE(state->usedTrailMaskName == "Image.candidate");
    }

    SECTION("no time slot") {
        masks.try_emplace("Text.candidate", "candidate", TrailMaskType::TEXT);
        scene.setAssociatedTexts(masks, {"Text.candidate"});
        REQUIRE(scene.applyRandomPresets(state));
        REQUIRE(state->usedTrailMaskName == "Text.candidate");
    }

    SECTION("no associated masks") {
        REQUIRE_FALSE(scene.applyRandomPresets(state));
        REQUIRE(state->usedTrailMaskName == "Image.previous");
    }

    SECTION("active animation is not interrupted") {
        masks.at("Image.previous").animation = TrailMaskAnimation{
            phys::Vec2<float>{0.0f, 0.0f}, phys::Vec2<float>{1.0f, 0.0f}, 10.0f};
        state->universalShaderSettings.trailMaskPosition = toStd140(phys::Vec2<float>{0.0f, 0.0f});
        properties.timeSlot = TimeSlot{now - std::chrono::hours{48}, now - std::chrono::hours{24}};
        masks.try_emplace("Image.candidate", "candidate", TrailMaskType::IMAGE, properties);
        scene.setAssociatedImages(masks, {"Image.candidate"});
        REQUIRE_FALSE(scene.applyRandomPresets(state));
        REQUIRE(state->usedTrailMaskName == "Image.previous");
    }

    SECTION("invalid selection is not retried when another mask is valid") {
        properties.timeSlot = TimeSlot{now - std::chrono::hours{48}, now - std::chrono::hours{24}};
        masks.try_emplace("Image.expired", "expired", TrailMaskType::IMAGE, properties);
        masks.try_emplace("Text.valid", "valid", TrailMaskType::TEXT);
        scene.setAssociatedImages(masks, {"Image.expired"});
        scene.setAssociatedTexts(masks, {"Text.valid"});

        int expiredSelections = 0;
        int validSelections = 0;
        for(unsigned int seed = 0; seed < 100; ++seed) {
            std::srand(seed);
            const int selectedIndex = std::rand() % 2;
            std::srand(seed);
            state->usedTrailMaskName = "Image.previous";
            REQUIRE(scene.applyRandomPresets(state));
            if(selectedIndex == 0) {
                ++expiredSelections;
                REQUIRE(state->usedTrailMaskName == kEmptyTrailMaskKey);
            } else {
                ++validSelections;
                REQUIRE(state->usedTrailMaskName == "Text.valid");
            }
        }
        REQUIRE(expiredSelections > 0);
        REQUIRE(validSelections > 0);
    }
}

TEST_CASE("Scene controller notifies when None is selected", "[scene][trail-mask]") {
    StateFixture fixture;
    TemporaryDirectory directory;
    std::ofstream(directory.path / "scenes.toml") << "";

    auto* state = fixture.state;
    std::map<std::string, TrailMask> masks;
    std::map<std::string, BehaviorPreset> behaviors;
    std::map<std::string, ColorPreset> colors;
    masks.try_emplace(kEmptyTrailMaskKey, "_None_", TrailMaskType::EMPTY);
    masks.try_emplace("Image.previous", "previous", TrailMaskType::IMAGE);
    TrailMaskProperties properties;
    const auto now = std::chrono::system_clock::now();
    properties.timeSlot = TimeSlot{now - std::chrono::hours{48}, now - std::chrono::hours{24}};
    masks.try_emplace("Image.expired", "expired", TrailMaskType::IMAGE, properties);
    state->trailMasks = &masks;
    state->usedTrailMaskName = "Image.previous";
    state->behaviorPresets = &behaviors;
    state->colorPresets = &colors;

    MaskEventRecorder recorder;
    SceneController controller{directory.path / "scenes.toml", state};
    controller.createScene("test");
    controller.editScene(SceneEditData{"test", {}, {}, {"Image.expired"}, {}});
    controller.addObserver(EventType::IMAGE_PRESET_APPLY, &recorder);
    controller.applyScene("test");

    REQUIRE(state->usedTrailMaskName == kEmptyTrailMaskKey);
    REQUIRE(recorder.count == 1);
    REQUIRE(recorder.key == kEmptyTrailMaskKey);

    state->usedTrailMaskName = "Image.previous";
    state->autoSceneSwitching = true;
    state->sceneSwitchingIntervall = 10;
    state->beatVolumeSwitch = 5;
    state->slimeSettings.velocityBassReaction = 6.0f;
    controller.autoSwitchScenes(10);
    REQUIRE(state->usedTrailMaskName == kEmptyTrailMaskKey);
    REQUIRE(recorder.count == 2);
    REQUIRE(recorder.key == kEmptyTrailMaskKey);
    controller.autoSwitchScenes(11);
}

TEST_CASE("None binds a neutral texture and disables mask influence", "[.opengl]") {
    if(!SDL_Init(SDL_INIT_VIDEO)) {
        SKIP("SDL video unavailable");
    }
    struct SDLGuard {
        ~SDLGuard() { SDL_Quit(); }
    } sdlGuard;
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    std::unique_ptr<SDL_Window, decltype(&SDL_DestroyWindow)> window{
        SDL_CreateWindow("Trail mask test", 32, 32, SDL_WINDOW_OPENGL | SDL_WINDOW_HIDDEN),
        SDL_DestroyWindow};
    if(!window) {
        SKIP("OpenGL window unavailable");
    }
    SDL_GLContext context = SDL_GL_CreateContext(window.get());
    if(!context) {
        SKIP("OpenGL context unavailable");
    }
    struct ContextGuard {
        SDL_GLContext context;
        ~ContextGuard() { SDL_GL_DestroyContext(context); }
    } contextGuard{context};
    glewExperimental = GL_TRUE;
    REQUIRE(glewInit() == GLEW_OK);

    StateFixture fixture;
    TemporaryDirectory directory;
    std::ofstream(directory.path / "timetable.toml") << "";
    FilePaths paths{
        .timeTableFilePath = directory.path / "timetable.toml",
        .pictureDir = directory.path,
        .pictureFileExtension = ".notpng",
        .fontFileDir = std::filesystem::path{TEST_RESOURCE_DIR} / "fonts",
        .fontAtlasTestOutputImage = directory.path / "fontAtlas.png"
    };
    auto* state = fixture.state;
    TrailMapController controller{&paths, 16, state};
    auto& masks = *state->trailMasks;
    REQUIRE(state->usedTrailMaskName == kEmptyTrailMaskKey);
    REQUIRE(state->universalShaderSettings.trailMaskInfluence == Catch::Approx(0.0f));
    REQUIRE_FALSE(state->universalShaderSettings.trailMaskIsInverted);
    controller.bindToTextureUnit(16);
    GLint initialTexture = 0;
    glGetIntegerv(GL_TEXTURE_BINDING_2D, &initialTexture);
    REQUIRE(static_cast<GLuint>(initialTexture) == masks.at(kEmptyTrailMaskKey).texture->getID());

    auto& mask = masks.try_emplace("Image.test", "test", TrailMaskType::IMAGE).first->second;
    TextureProperties textureProperties;
    textureProperties.width = 1;
    textureProperties.height = 1;
    textureProperties.generateMipmaps = false;
    textureProperties.minFilter = TextureMinFilter::NEAREST;
    textureProperties.magFilter = TextureMagFilter::NEAREST;
    const float whitePixel[4] = {1.0f, 1.0f, 1.0f, 1.0f};
    mask.texture = std::make_unique<Texture>(textureProperties, whitePixel,
        TextureDataFormat::RGBA, TextureDataType::FLOAT, sizeof(whitePixel));
    mask.isInverted = true;
    state->usedTrailMaskName = "Image.test";
    controller.onNotify(UserEvent{EventType::IMAGE_PRESET_APPLY});
    REQUIRE(state->universalShaderSettings.trailMaskInfluence ==
            Catch::Approx(mask.strength + TrailMapController::globalStrength_));
    REQUIRE(state->universalShaderSettings.trailMaskIsInverted);

    std::ofstream(directory.path / "scenes.toml") << "";
    std::map<std::string, BehaviorPreset> behaviors;
    std::map<std::string, ColorPreset> colors;
    state->behaviorPresets = &behaviors;
    state->colorPresets = &colors;
    const auto now = std::chrono::system_clock::now();
    mask.timeSlot = TimeSlot{now - std::chrono::hours{48}, now - std::chrono::hours{24}};
    SceneController sceneController{directory.path / "scenes.toml", state};
    sceneController.createScene("test");
    sceneController.editScene(SceneEditData{"test", {}, {}, {"Image.test"}, {}});
    sceneController.addObserver(EventType::IMAGE_PRESET_APPLY, &controller);
    state->autoSceneSwitching = true;
    state->sceneSwitchingIntervall = 10;
    state->beatVolumeSwitch = 5;
    state->slimeSettings.velocityBassReaction = 6.0f;
    sceneController.autoSwitchScenes(20);
    sceneController.autoSwitchScenes(21);
    REQUIRE(state->usedTrailMaskName == kEmptyTrailMaskKey);
    REQUIRE(state->universalShaderSettings.trailMaskInfluence == Catch::Approx(0.0f));
    REQUIRE_FALSE(state->universalShaderSettings.trailMaskIsInverted);

    for(GLuint unit : {5u, 16u}) {
        controller.bindToTextureUnit(unit);
        GLint boundTexture = 0;
        glGetIntegerv(GL_TEXTURE_BINDING_2D, &boundTexture);
        REQUIRE(static_cast<GLuint>(boundTexture) == masks.at(kEmptyTrailMaskKey).texture->getID());
        float pixel[4] = {};
        glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_FLOAT, pixel);
        REQUIRE(pixel[0] == Catch::Approx(0.0f));
        REQUIRE(pixel[1] == Catch::Approx(0.0f));
        REQUIRE(pixel[2] == Catch::Approx(0.0f));
        REQUIRE(pixel[3] == Catch::Approx(0.0f));
    }

    struct StrengthGuard {
        float original = TrailMapController::globalStrength_;
        ~StrengthGuard() { TrailMapController::globalStrength_ = original; }
    } strengthGuard;
    TrailMapController::globalStrength_ = 3.0f;
    controller.onNotify(UserEvent{EventType::IMAGE_PRESET_APPLY});
    REQUIRE(state->universalShaderSettings.trailMaskInfluence == Catch::Approx(0.0f));
    state->usedTrailMaskName = "Image.test";
    controller.onNotify(UserEvent{EventType::IMAGE_PRESET_APPLY});
    REQUIRE(state->universalShaderSettings.trailMaskInfluence ==
            Catch::Approx(mask.strength + TrailMapController::globalStrength_));
    REQUIRE(state->universalShaderSettings.trailMaskIsInverted);
    state->usedTrailMaskName = kEmptyTrailMaskKey;
    controller.onNotify(UserEvent{EventType::TEXT_PRESET_APPLY});
    REQUIRE(state->universalShaderSettings.trailMaskInfluence == Catch::Approx(0.0f));
    REQUIRE_FALSE(state->universalShaderSettings.trailMaskIsInverted);
}
