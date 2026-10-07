#include "application.h"

#include "utility/event.h"
#include "utility/parameter_parser.h"

Application::Application(Parameters params) 
 :  appState_{ApplicationState::getInstance(params)},
	window_{Window("Physarum", appState_, params.customResolution)},
	inputHandler_{InputHandler(appState_)},
	ui_{UserInterface(window_.getWindow(), window_.getGLContext(), filePaths_, appState_)},
	ubo_manager_{UniformBufferManager(appState_)},
	simulation_{Simulation(&ubo_manager_, &filePaths_, appState_, params.customParticleCount)},
	renderer_{std::make_unique<Renderer>(&ubo_manager_, &filePaths_, appState_)},
	audioSystem_{AudioSystem(appState_)},
	presetSystem_{PresetSystem<BehaviorPreset>(filePaths_.behaviorPresetFilePath, appState_)},
	colorPresetSystem_{PresetSystem<ColorPreset>(filePaths_.colorPresetFilePath, appState_)},
	sceneController_{SceneController(filePaths_.sceneFilePath, appState_)},
	musicAnalysis_{MusicAnalysis(appState_)}
{
	//------------------------------------------------------
	// Register Observers for immediate reaction to Events

	setUpObservers();	// Implementation in application_setup.cpp !

	
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

		//Auto Switching Scenes
		sceneController_.autoSwitchScenes(timeInSeconds);
	}
}