#include "application.h"

Application::Application(Parameters params) 
 :  appState_{ApplicationState::getInstance(params)},
	window_{Window("Physarum", appState_, params.customResolution)},
	inputHandler_{InputHandler(appState_)},
	ui_{UserInterface(window_.getWindow(), window_.getGLContext(), appState_)},
	ubo_manager_{UniformBufferManager(appState_)},
	simulation_{Simulation(&ubo_manager_, &ui_, params.customParticleCount)},
	renderer_{std::make_unique<Renderer>(&ubo_manager_, appState_)},
	audioSystem_{AudioSystem(appState_, params.audioDevice)},
	presetSystem_{PresetSystem("./presets/", ".psf", &ui_)},
	colorPresetSystem_{ColorPresetSystem("./presets/", ".pcsf", &ui_)},
	musicAnalysis_{MusicAnalysis(appState_)}
{
	//------------------------------------------------------
	//Register Observers for immediate reaction to Events
	ui_.getWindow("VisualSettingsWindow")->addObserver(Event::FULLSCREEN_TOGGLE, &window_);
	ui_.getWindow("AudioWindow")->addObserver(Event::AUDIO_HARDWARE_CHANGE, &audioSystem_);
	ui_.getWindow("PresetWindow")->addObserver(Event::SAVE_PRESET, &presetSystem_);
	ui_.getWindow("PresetWindow")->addObserver(Event::LOAD_PRESET, &presetSystem_);
	ui_.getWindow("PresetWindow")->addObserver(Event::SAVE_COLOR_PRESET, &colorPresetSystem_);
	ui_.getWindow("PresetWindow")->addObserver(Event::LOAD_COLOR_PRESET, &colorPresetSystem_);
	ui_.getWindow("PresetWindow")->addObserver(Event::LOAD_NEW_PICTURE, simulation_.getTrailMapController());
	ui_.getWindow("NewCanvasModal")->addObserver(Event::NEW_CANVAS, renderer_.get());
	ui_.getWindow("NewCanvasModal")->addObserver(Event::NEW_CANVAS, &simulation_);
	ui_.getWindow("NewCanvasModal")->addObserver(Event::NEW_CANVAS, &ubo_manager_);

	//------------------------------------------------------
	// Initialize Audio Recording and Processing
	std::vector<std::string> availableAudioHardwareNames = audioSystem_.getAvailableHardwareDeviceNames();
	audioWindow_ = dynamic_cast<AudioWindow*>(ui_.getWindow("AudioWindow"));
	audioWindow_->addHardwareDeviceNames(availableAudioHardwareNames);

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
		inputHandler_.processUserInput();
		window_.processWindowEvents();

		//------------------------------------------------------
		// setting Uniforms for later use in Draw Call
		ubo_manager_.updateUBOs();

		//------------------------------------------------------
		// Compute Shader Passes for Simulation Steps
		simulation_.simulateStep();

		//------------------------------------------------------
		// Audio Processing
		if(appState_->slimeSettings.reactToAudio) {
			audioSystem_.computeSpectrum();
			musicAnalysis_.analyzeMusic(audioSystem_.getSpectrumDiff(), frameTime);
		}
		
		//------------------------------------------------------
		// Draw Call with Rasterization Pipeline
		renderer_->draw();
		
		//------------------------------------------------------
		// ImGui Draw Call
		ui_.display(audioSystem_.getAudioBuffer(), audioSystem_.getSpectrum(), audioSystem_.getSpectrumDiff(), audioSystem_.getBufferSize(), audioSystem_.hasNewSpectrumData());
		audioSystem_.setHasNewSpectrumData(false);
		
		//------------------------------------------------------
		// Swap draw buffers with SDL3
		window_.swapBuffers();

		//Auto Switching Presets
		presetSystem_.autoSwitchPresets(&ui_, timeInSeconds);
		colorPresetSystem_.autoSwitchPresets(&ui_, timeInSeconds);
		simulation_.getTrailMapController()->autoSwitchPictures(&ui_, timeInSeconds);
	}
}