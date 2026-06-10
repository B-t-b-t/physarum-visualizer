#include "application.h"

Application::Application(Parameters params) 
  :	params_{params}, 
	workGroupDivider_{params.workGroupDivider}, 
	window_{Window((int)params.width, (int)params.height, "Physarum", params.customResolution)},
	ui_{UserInterface(window_.getWindow(), window_.getGLContext())},
	uiState_{ui_.getState()},
	ui_uss_{uiState_->universalShaderSettings},	//just shortening the name as a temp solution
	trailDiffusionShader_{Shader("./res/TrailDiffusion.cs", ShaderType::COMPUTE_SHADER)},
	trailDiffusionProgram_{ShaderProgram("TrailDiffusionProgram", {&trailDiffusionShader_})},
	particleBehaviourShader_{Shader("./res/ParticleBehaviour.cs", ShaderType::COMPUTE_SHADER)},
	particleBehaviourProgram_{ShaderProgram("ParticleBehaviourProgram", {&particleBehaviourShader_})},
	audioSystem_{AudioSystem(params.audioDevice)},
	presetSystem_{PresetSystem("./presets/", ".psf", &ui_)},
	colorPresetSystem_{ColorPresetSystem("./presets/", ".pcsf", &ui_)},
	trailMapController_{TrailMapController("./res/pictures/", ".png", 16, &ui_)},	//Texture Unit 16 for Trail Mask Texture
	musicAnalysis_{MusicAnalysis(uiState_)}
{
	//------------------------------------------------------
	//beginn constructor body

	//Register Observers for Events
	window_.attachToObservable(Event::FULLSCREEN_TOGGLE, ui_.getWindow("VisualSettingsWindow"));
	audioSystem_.attachToObservable(Event::AUDIO_HARDWARE_CHANGE, ui_.getWindow("AudioWindow"));
	presetSystem_.attachToObservable(Event::SAVE_PRESET, ui_.getWindow("PresetWindow"));
	presetSystem_.attachToObservable(Event::LOAD_PRESET, ui_.getWindow("PresetWindow"));
	colorPresetSystem_.attachToObservable(Event::SAVE_COLOR_PRESET, ui_.getWindow("PresetWindow"));
	colorPresetSystem_.attachToObservable(Event::LOAD_COLOR_PRESET, ui_.getWindow("PresetWindow"));
	trailMapController_.attachToObservable(Event::LOAD_NEW_PICTURE, ui_.getWindow("PresetWindow"));

	//update window size because the window is created in window constructor with maximum possible size for the screen
	ui_uss_.windowWidth = window_.getWindowWidth();
	ui_uss_.windowHeight = window_.getWindowHeight();
	uiState_->numParticles = params_.numParticles;
	uiState_->newNumParticles = params_.numParticles;
	uiState_->slimeRatio = params_.slimeRatio;

	//calculate texture size based on window size and fractional scaling factor for high DPI displays
	//texture size should reflect resolution of the physical display and not the scaled OS resolution
	float scaling = window_.getFractionalScalingFactor();

	ui_uss_.textureWidth = (int)(ui_uss_.windowWidth * scaling - ((int)(ui_uss_.windowWidth * scaling) % workGroupDivider_));		//make sure width is multiple of workgroup size
	ui_uss_.textureHeight = (int)(ui_uss_.windowHeight * scaling - ((int)(ui_uss_.windowHeight * scaling) % workGroupDivider_));	//make sure height is multiple of workgroup size
	uiState_->newTextureWidth = ui_uss_.textureWidth;
	uiState_->newTextureHeight = ui_uss_.textureHeight;

	//calculate number of particles based on texture size and slime ratio
	if(params_.customParticleCount) {
		uiState_->slimeRatio = uiState_->numParticles / (float)(ui_uss_.textureWidth * ui_uss_.textureHeight);
	} else {
		uiState_->numParticles = uiState_->slimeRatio * ui_uss_.textureWidth * ui_uss_.textureHeight;
		uiState_->numParticles = uiState_->numParticles - uiState_->numParticles % workGroupDivider_;	//ensure multiple of workgroup size
	}

	renderer_ = Renderer();	//initialize after setting the correct texture size because of false texture initialization in Renderer Constructor??
	
	//------------------------------------------------------
	// Initialize Physarum Particles
	particleData_.createAndSend(uiState_->numParticles, ui_uss_.textureWidth, ui_uss_.textureHeight);
	
	//------------------------------------------------------
	// Initialize Audio Recording and Processing
	std::vector<std::string> availableAudioHardwareNames = audioSystem_.getAvailableHardwareDeviceNames();
	audioWindow_ = dynamic_cast<AudioWindow*>(ui_.getWindow("AudioWindow"));
	audioWindow_->addHardwareDeviceNames(availableAudioHardwareNames);
	
    universalShaderSettingsUBO_ = UniformBufferObject(0);
	universalShaderSettingsUBO_.bindUniformBufferObject(ui_uss_);
	slimeSettingsUBO_ = UniformBufferObject(1);
	slimeSettingsUBO_.bindUniformBufferObject(uiState_->slimeSettings);
	trailDiffusionUBO_ = UniformBufferObject(2);
	trailDiffusionUBO_.bindUniformBufferObject(uiState_->trailDiffusionSettings);
	fragmentShaderSettingsUBO_ = UniformBufferObject(3);
	fragmentShaderSettingsUBO_.bindUniformBufferObject(uiState_->fragmentShaderSettings);
	parameterSettingsUBO_ = UniformBufferObject(4);
	parameterSettingsUBO_.bindUniformBufferObject(uiState_->parameterSettings);
	
	particleBehaviourProgram_.attachUniformBufferObject(universalShaderSettingsUBO_.getUniformBufferObjectID(), "UniversalShaderSettings", 0);
	particleBehaviourProgram_.attachUniformBufferObject(slimeSettingsUBO_.getUniformBufferObjectID(), "SlimeSettings", 1);
	particleBehaviourProgram_.attachUniformBufferObject(parameterSettingsUBO_.getUniformBufferObjectID(), "ParameterSettings", 4);
	
	trailDiffusionProgram_.attachUniformBufferObject(universalShaderSettingsUBO_.getUniformBufferObjectID(), "UniversalShaderSettings", 0);
	trailDiffusionProgram_.attachUniformBufferObject(trailDiffusionUBO_.getUniformBufferObjectID(), "TrailDiffusionSettings", 2);
	
	renderer_.attachUniformBufferObject(universalShaderSettingsUBO_.getUniformBufferObjectID(), "UniversalShaderSettings", 0);
	renderer_.attachUniformBufferObject(fragmentShaderSettingsUBO_.getUniformBufferObjectID(), "FragmentShaderSettings", 3);

	//resize textures to correct size according to window size and scaling factor
	renderer_.resizeTextures(ui_uss_.textureWidth, ui_uss_.textureHeight);

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

		ui_uss_.timeTicks = nowCounter;

		//------------------------------------------------------
		// setting Uniforms for later use in Draw Call
		universalShaderSettingsUBO_.updateUniformBufferObject(ui_uss_);
		slimeSettingsUBO_.updateUniformBufferObject(uiState_->slimeSettings);
		trailDiffusionUBO_.updateUniformBufferObject(uiState_->trailDiffusionSettings);
		fragmentShaderSettingsUBO_.updateUniformBufferObject(uiState_->fragmentShaderSettings);
		parameterSettingsUBO_.updateUniformBufferObject(uiState_->parameterSettings);

		//------------------------------------------------------
		// Compute Shader Passes for Simulation Steps

		trailMapController_.bindToTextureUnit(5);
		trailDiffusionProgram_.dispatchCompute(ui_uss_.textureWidth / workGroupDivider_, ui_uss_.textureHeight / workGroupDivider_, 1);	//calculate new trail texture
		particleBehaviourProgram_.dispatchCompute(uiState_->numParticles / 8, (GLuint)1, 1);	//move Slime Particles

		//------------------------------------------------------
		// Audio Processing
		if(uiState_->slimeSettings.reactToAudio) {
			audioSystem_.computeSpectrum();
		}

		//give UI access to audio data; Why Here?!
		ui_.display(audioSystem_.getAudioBuffer(), audioSystem_.getSpectrum(), audioSystem_.getSpectrumDiff(), audioSystem_.getBufferSize(), audioSystem_.hasNewSpectrumData());
		audioSystem_.setHasNewSpectrumData(false);

		// Analyze the music data
		if(uiState_->slimeSettings.reactToAudio) {
			musicAnalysis_.analyzeMusic(audioSystem_.getSpectrumDiff(), frameTime);
		}

		trailMapController_.bindToTextureUnit(16);

		//------------------------------------------------------
		// Draw Call with Rasterization Pipeline
		renderer_.draw();

		//------------------------------------------------------
		// ImGui Draw Call
		glBindFramebuffer(GL_FRAMEBUFFER, 0); // Default framebuffer to make sure
		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
		
		//------------------------------------------------------
		//process Key Presses from User
		window_.Update();	//TODO: badly named; RENAME!

		if(window_.getExitLock()) { 				//if exit lock is active go to fullscreen
			uiState_->fullscreen = true;
		}
		ui_uss_.windowWidth = window_.getWindowWidth();		//synchronize window size
		ui_uss_.windowHeight = window_.getWindowHeight();

		//------------------------------------------------------
		// Handle User Interface Changes that affect Simulation State
		if(uiState_->newCanvas) {
			std::cout << "Creating new Canvas with " << uiState_->numParticles << " particles and size " << ui_uss_.textureWidth << "x" << ui_uss_.textureHeight << std::endl;
			if(uiState_->newTextureWidth != ui_uss_.textureWidth || uiState_->newTextureHeight != ui_uss_.textureHeight) {
				// Resize Textures and Framebuffers
				ui_uss_.textureWidth = uiState_->newTextureWidth;
				ui_uss_.textureHeight = uiState_->newTextureHeight;

				renderer_.resizeTextures(ui_uss_.textureWidth, ui_uss_.textureHeight);
			}

			particleData_.createAndSend(uiState_->numParticles, ui_uss_.textureWidth, ui_uss_.textureHeight);
			universalShaderSettingsUBO_.updateUniformBufferObject(ui_uss_);
			uiState_->newCanvas = false;
		}

		//Auto Switching Presets
		presetSystem_.autoSwitchPresets(&ui_, timeInSeconds);
		colorPresetSystem_.autoSwitchPresets(&ui_, timeInSeconds);
		trailMapController_.autoSwitchPictures(&ui_, timeInSeconds);

		//Exit Program when requested from UI
		if(uiState_->exitProgram) {
			if(!window_.setIsClosed(true)) {
				uiState_->exitProgram = false;
			}
		}
	}
}