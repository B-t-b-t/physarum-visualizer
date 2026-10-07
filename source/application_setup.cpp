#include "application.h"

//Part of Application Class
//------------------------------------------------------
//Register Observers for immediate reaction to Events
void Application::setUpObservers() {

    // User Request for Fullscreen through UI
	ui_.getWindow("VisualSettingsWindow")->addObserver(EventType::FULLSCREEN_TOGGLE, &window_);

    // User Request for Audio Hardware changes through the AudioWindow
	ui_.getWindow("AudioWindow")->addObserver(EventType::AUDIO_HARDWARE_CHANGE, &audioSystem_);

    // User Interactions with Presets through the PresetWindow
	ui_.getWindow("PresetWindow")->addObserver(EventType::BEHAVIOR_PRESET_CREATE, &presetSystem_);
	ui_.getWindow("PresetWindow")->addObserver(EventType::BEHAVIOR_PRESET_APPLY, &presetSystem_);
	ui_.getWindow("PresetWindow")->addObserver(EventType::BEHAVIOR_PRESET_DELETE, &presetSystem_);
	ui_.getWindow("PresetWindow")->addObserver(EventType::COLOR_PRESET_CREATE, &colorPresetSystem_);
	ui_.getWindow("PresetWindow")->addObserver(EventType::COLOR_PRESET_APPLY, &colorPresetSystem_);
	ui_.getWindow("PresetWindow")->addObserver(EventType::COLOR_PRESET_DELETE, &colorPresetSystem_);
	ui_.getWindow("PresetWindow")->addObserver(EventType::IMAGE_PRESET_APPLY, simulation_.getTrailMapController());
	ui_.getWindow("PresetWindow")->addObserver(EventType::IMAGE_PRESET_EDIT, simulation_.getTrailMapController());
	ui_.getWindow("PresetWindow")->addObserver(EventType::TEXT_PRESET_APPLY, simulation_.getTrailMapController());	
	ui_.getWindow("PresetWindow")->addObserver(EventType::TEXT_PRESET_CREATE, simulation_.getTrailMapController());
	ui_.getWindow("PresetWindow")->addObserver(EventType::TEXT_PRESET_EDIT, simulation_.getTrailMapController());
	ui_.getWindow("PresetWindow")->addObserver(EventType::TEXT_PRESET_DELETE, simulation_.getTrailMapController());

    // User Requests, that modify the GPU buffers (change of canvas texture resolution, number of particles, ...)
	ui_.getWindow("NewCanvasModal")->addObserver(EventType::NEW_CANVAS, renderer_.get());
	ui_.getWindow("NewCanvasModal")->addObserver(EventType::NEW_CANVAS, &simulation_);
	ui_.getWindow("NewCanvasModal")->addObserver(EventType::NEW_CANVAS, &ubo_manager_);

    // User Interactions with Scenes through the ScenesWindow
	ui_.getWindow("ScenesWindow")->addObserver(EventType::SCENE_APPLY, &sceneController_);
	ui_.getWindow("ScenesWindow")->addObserver(EventType::SCENE_CREATE, &sceneController_);
	ui_.getWindow("ScenesWindow")->addObserver(EventType::SCENE_DELETE, &sceneController_);
	ui_.getWindow("ScenesWindow")->addObserver(EventType::SCENE_EDIT, &sceneController_);

    // Scene Controller cannot apply presets directly, it must do this through the respective preset systems
	sceneController_.addObserver(EventType::BEHAVIOR_PRESET_APPLY, &presetSystem_);
	sceneController_.addObserver(EventType::COLOR_PRESET_APPLY, &colorPresetSystem_);
	sceneController_.addObserver(EventType::IMAGE_PRESET_APPLY, simulation_.getTrailMapController());
	sceneController_.addObserver(EventType::TEXT_PRESET_APPLY, simulation_.getTrailMapController());

    // If a preset is deleted by the user, the associated presets in each scene should be updated accordingly
    ui_.getWindow("PresetWindow")->addObserver(EventType::BEHAVIOR_PRESET_DELETE, &sceneController_);
    ui_.getWindow("PresetWindow")->addObserver(EventType::COLOR_PRESET_DELETE, &sceneController_);
    ui_.getWindow("PresetWindow")->addObserver(EventType::IMAGE_PRESET_DELETE, &sceneController_);
    ui_.getWindow("PresetWindow")->addObserver(EventType::TEXT_PRESET_DELETE, &sceneController_);
}