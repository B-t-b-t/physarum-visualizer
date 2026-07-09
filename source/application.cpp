#include "application.h"

Application::Application(Parameters params) 
  :	appState_{ApplicationState::getInstance(params)},
	window_{Window("Physarum", appState_, params.customResolution)},
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
	//Register Observers for Events
	window_.attachToObservable(Event::FULLSCREEN_TOGGLE, ui_.getWindow("VisualSettingsWindow"));
	audioSystem_.attachToObservable(Event::AUDIO_HARDWARE_CHANGE, ui_.getWindow("AudioWindow"));
	presetSystem_.attachToObservable(Event::SAVE_PRESET, ui_.getWindow("PresetWindow"));
	presetSystem_.attachToObservable(Event::LOAD_PRESET, ui_.getWindow("PresetWindow"));
	colorPresetSystem_.attachToObservable(Event::SAVE_COLOR_PRESET, ui_.getWindow("PresetWindow"));
	colorPresetSystem_.attachToObservable(Event::LOAD_COLOR_PRESET, ui_.getWindow("PresetWindow"));
	simulation_.getTrailMapController()->attachToObservable(Event::LOAD_NEW_PICTURE, ui_.getWindow("PresetWindow"));
	renderer_->attachToObservable(Event::NEW_CANVAS, ui_.getWindow("NewCanvasModal"));
	simulation_.attachToObservable(Event::NEW_CANVAS, ui_.getWindow("NewCanvasModal"));
	ubo_manager_.attachToObservable(Event::NEW_CANVAS, ui_.getWindow("NewCanvasModal"));
	
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
	while (!window_.IsClosed()) {

		Uint64 nowCounter = SDL_GetPerformanceCounter();
		Uint64 timeInSeconds = nowCounter / counterFrequency_; //SDL Timer in Seconds for Audio Beat Analysis and Auto Preset Switching
		double frameTime = double(nowCounter - prevCounter_) / double(counterFrequency_);
		prevCounter_ = nowCounter;

		appState_->universalShaderSettings.timeTicks = nowCounter;

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
		}

		
		// Analyze the music data
		if(appState_->slimeSettings.reactToAudio) {
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
		//process Key Presses from User
		window_.Update();	//TODO: badly named; RENAME!

		if(window_.getExitLock()) { 				//if exit lock is active go to fullscreen
			appState_->fullscreen = true;
		}

		//Auto Switching Presets
		presetSystem_.autoSwitchPresets(&ui_, timeInSeconds);
		colorPresetSystem_.autoSwitchPresets(&ui_, timeInSeconds);
		simulation_.getTrailMapController()->autoSwitchPictures(&ui_, timeInSeconds);

		//Exit Program when requested from UI
		if(appState_->exitProgram) {
			if(!window_.setIsClosed(true)) {
				appState_->exitProgram = false;
			}
		}
	}
}