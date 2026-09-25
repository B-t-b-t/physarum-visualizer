#include "application.h"

#include "utility/event.h"
#include "utility/parameter_parser.h"

Application::Application(Parameters params) 
 :  appState_{ApplicationState::getInstance(params)},
	window_{Window("Physarum", appState_, params.customResolution)},
	inputHandler_{InputHandler(appState_)},
	ui_{UserInterface(window_.getWindow(), window_.getGLContext(), appState_)},
	ubo_manager_{UniformBufferManager(appState_)},
	simulation_{Simulation(&ubo_manager_, appState_, params.customParticleCount)},
	renderer_{std::make_unique<Renderer>(&ubo_manager_, appState_)},
	audioSystem_{AudioSystem(appState_)},
	presetSystem_{PresetSystem<BehaviorPreset>("./presets/behaviourPresets.toml", appState_)},
	colorPresetSystem_{PresetSystem<ColorPreset>("./presets/colorPresets.toml", appState_)},
	musicAnalysis_{MusicAnalysis(appState_)}
{
	//------------------------------------------------------
	//Register Observers for immediate reaction to Events
	ui_.getWindow("VisualSettingsWindow")->addObserver(EventType::FULLSCREEN_TOGGLE, &window_);
	ui_.getWindow("AudioWindow")->addObserver(EventType::AUDIO_HARDWARE_CHANGE, &audioSystem_);
	ui_.getWindow("PresetWindow")->addObserver(EventType::BEHAVIOR_PRESET_CREATE, &presetSystem_);
	ui_.getWindow("PresetWindow")->addObserver(EventType::BEHAVIOR_PRESET_APPLY, &presetSystem_);
	ui_.getWindow("PresetWindow")->addObserver(EventType::BEHAVIOR_PRESET_DELETE, &presetSystem_);
	ui_.getWindow("PresetWindow")->addObserver(EventType::COLOR_PRESET_CREATE, &colorPresetSystem_);
	ui_.getWindow("PresetWindow")->addObserver(EventType::COLOR_PRESET_APPLY, &colorPresetSystem_);
	ui_.getWindow("PresetWindow")->addObserver(EventType::COLOR_PRESET_DELETE, &colorPresetSystem_);
	ui_.getWindow("PresetWindow")->addObserver(EventType::IMAGE_PRESET_APPLY, simulation_.getTrailMapController());
	ui_.getWindow("PresetWindow")->addObserver(EventType::TEXT_PRESET_CREATE, simulation_.getTrailMapController());
	ui_.getWindow("PresetWindow")->addObserver(EventType::TEXT_PRESET_EDIT, simulation_.getTrailMapController());
	ui_.getWindow("PresetWindow")->addObserver(EventType::TEXT_PRESET_DELETE, simulation_.getTrailMapController());
	ui_.getWindow("PresetWindow")->addObserver(EventType::EDIT_TRAIL_MASK_TIME_SLOT, simulation_.getTrailMapController());
	ui_.getWindow("PresetWindow")->addObserver(EventType::TRAIL_MASK_STRENGTH_CHANGED, simulation_.getTrailMapController());
	ui_.getWindow("NewCanvasModal")->addObserver(EventType::NEW_CANVAS, renderer_.get());
	ui_.getWindow("NewCanvasModal")->addObserver(EventType::NEW_CANVAS, &simulation_);
	ui_.getWindow("NewCanvasModal")->addObserver(EventType::NEW_CANVAS, &ubo_manager_);

	prevCounter_ = SDL_GetPerformanceCounter();
	counterFrequency_ = SDL_GetPerformanceFrequency(); //SDL Timer Frequency for Audio Beat Analysis and Auto Preset Switching
}


void Application::run() {
	//======================================================================
	// Main Loop
	//======================================================================
	while (!window_.isClosing()) {

		Uint64 nowCounter = SDL_GetPerformanceCounter();
		Uint64 timeInSeconds = nowCounter / counterFrequency_; //SDL Timer in Seconds for Audio Beat Analysis and Auto Preset Switching
		double frameTime = double(nowCounter - prevCounter_) / double(counterFrequency_);
		prevCounter_ = nowCounter;

		appState_->universalShaderSettings.timeTicks = nowCounter;

		//------------------------------------------------------
		// handle user input through keyboard, mouse and window
		SDL_Event event;
		while (SDL_PollEvent(&event)) {

			//just for X11 enable mouse capture, otherwise ImGui windows lose mouse focus when resizing or moving window
			//issues like stuttering, losing focus, lost mouse states,...
			//!disable when debugging on X11, otherwise it leads to mouse issues when breaking while mouse is captured!
			ImGui_ImplSDL3_SetMouseCaptureMode(ImGui_ImplSDL3_MouseCaptureMode_Enabled);

			ImGui_ImplSDL3_ProcessEvent(&event);	//give ImGui access to SDL3 input and let it decide if it needs to capture the input
			window_.processWindowEvents(&event);	//always process window events

			//don't process user input if the UI wants to capture it
			if(!ui_.wantsInput()) {
				inputHandler_.processUserInput(&event, window_.getWindow());
			} else {
				inputHandler_.cleanUpInput();
			}
		}

		//------------------------------------------------------
		// setting Uniforms for later use in Draw Call
		ubo_manager_.updateUBOs();

		//------------------------------------------------------
		// Compute Shader Passes for Simulation Steps
		simulation_.simulateStep();

		//------------------------------------------------------
		// Audio Processing
		audioSystem_.update();

		if(appState_->slimeSettings.reactToAudio) {
			audioSystem_.computeSpectrum();
			musicAnalysis_.analyzeMusic(audioSystem_.getSpectrumDiff(), frameTime);
		}
		
		//------------------------------------------------------
		// Draw Call with Rasterization Pipeline
		renderer_->draw();
		
		//------------------------------------------------------
		// ImGui Draw Call
		ui_.display();
		
		//------------------------------------------------------
		// Swap draw buffers with SDL3
		window_.swapBuffers();

		//Auto Switching Presets
		presetSystem_.autoSwitchPresets(timeInSeconds);
		colorPresetSystem_.autoSwitchPresets(timeInSeconds);
		simulation_.getTrailMapController()->autoSwitchPictures(timeInSeconds);
	}
}