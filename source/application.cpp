#include "application.h"

Application::Application(Parameters params, int workGroupDivider) 
  :	params_{params}, 
	workGroupDivider_{workGroupDivider}, 
	window_{Window((int)params.width, (int)params.height, "Physarum", params.customResolution)},
	ui_{UserInterface(window_.getWindow(), window_.getGLContext())},
	uiState_{ui_.getState()},
	ui_uss_{uiState_->universalShaderSettings},	//just shortening the name as a temp solution
	drawCanvas_{Canvas()},
	audioSystem_{AudioSystem(params.audioDevice)}
{
}

Application::~Application() {
    
}

void Application::initialize() {

	//------------------------------------------------------
	// Initialize Window
    //window_ = Window((int)params_.width, (int)params_.height, "Physarum", params_.customResolution);
	
	//------------------------------------------------------
	// Initialize User Interface
	//ui_ = UserInterface(window_.getWindow(), window_.getGLContext());
	//uiState_ = ui_.getState();

	//auto& ui_uss = uiState_->universalShaderSettings;	//just shortening the name as a temp solution
	
	ui_uss_.windowWidth = window_.getWindowWidth();
	ui_uss_.windowHeight = window_.getWindowHeight();
	uiState_->numParticles = params_.numParticles;
	uiState_->newNumParticles = params_.numParticles;
	uiState_->slimeRatio = params_.slimeRatio;

	float scaling = window_.getFractionalScalingFactor();

	ui_uss_.textureWidth = (int)(ui_uss_.windowWidth * scaling - ((int)(ui_uss_.windowWidth * scaling) % workGroupDivider_));		//make sure width is multiple of workgroup size
	ui_uss_.textureHeight = (int)(ui_uss_.windowHeight * scaling - ((int)(ui_uss_.windowHeight * scaling) % workGroupDivider_));	//make sure height is multiple of workgroup size
	uiState_->newTextureWidth = ui_uss_.textureWidth;
	uiState_->newTextureHeight = ui_uss_.textureHeight;

	if(params_.customParticleCount) {
		uiState_->slimeRatio = uiState_->numParticles / (float)(ui_uss_.textureWidth * ui_uss_.textureHeight);
	} else {
		uiState_->numParticles = uiState_->slimeRatio * ui_uss_.textureWidth * ui_uss_.textureHeight;
		uiState_->numParticles = uiState_->numParticles - uiState_->numParticles % workGroupDivider_;	//ensure multiple of workgroup size
	}

	//------------------------------------------------------
	//Initialize Textures and Texture Buffers
	texTrail_ = Texture((int)ui_uss_.textureWidth, (int)ui_uss_.textureHeight, Texture::TextureType::RGBA_FLOAT, 0);		//Texture Unit 0
	texTrailNonDiffused_ = Texture((int)ui_uss_.textureWidth, (int)ui_uss_.textureHeight, Texture::TextureType::RGBA_FLOAT, 1);	//Texture Unit 1
	newTexParticles_ = Texture((int)ui_uss_.textureWidth, (int)ui_uss_.textureHeight, Texture::TextureType::R_UINT, 2);		//Texture Unit 2
	oldTexParticles_ = Texture((int)ui_uss_.textureWidth, (int)ui_uss_.textureHeight, Texture::TextureType::R_UINT, 3);		//Texture Unit 3
	texCollisions_ = Texture((int)ui_uss_.textureWidth, (int)ui_uss_.textureHeight, Texture::TextureType::RGBA_FLOAT, 4);		//Texture Unit 4

	//------------------------------------------------------
	//initialize Rasterizer Pipeline for Canvas
	vertexShader_ = Shader("./res/vertex.vs", ShaderType::VERTEX_SHADER);
	fragmentShader_ = Shader("./res/fragment.fs", ShaderType::FRAGMENT_SHADER);

	rasterizationPipeline_ = ShaderProgram("RasterizationPipeline");
	rasterizationPipeline_.attachShader(vertexShader_.getShaderID());
	rasterizationPipeline_.attachShader(fragmentShader_.getShaderID());
	bool linkingSuccess = rasterizationPipeline_.link();
	if(!linkingSuccess) {
		std::cerr << "Failed to link Rasterization Pipeline Shader Program. Exiting." << std::endl;
		return;
	}
	rasterizationPipeline_.getUniformsFromGLSL();

	// Initialize Bloom Effect
	bloomEffect_ = Bloom(ui_uss_.textureWidth, ui_uss_.textureHeight, vertexShader_.getShaderID());

	//------------------------------------------------------
	// Initialize Physarum Particles
	particleData_.createAndSend(uiState_->numParticles, ui_uss_.textureWidth, ui_uss_.textureHeight);

	//------------------------------------------------------
	// Initialize Compute Shaders

	//initialize Compute Shader for Slime Mold Trail Texture
	trailDiffusionShader_ = Shader("./res/TrailDiffusion.cs", ShaderType::COMPUTE_SHADER);
	trailDiffusionProgram_ = ShaderProgram("TrailDiffusionProgram");
	trailDiffusionProgram_.attachShader(trailDiffusionShader_.getShaderID());
	linkingSuccess = trailDiffusionProgram_.link();
	if(!linkingSuccess) {
		std::cerr << "Failed to link Trail Diffusion Shader Program. Exiting." << std::endl;
		return;
	}
	trailDiffusionProgram_.getUniformsFromGLSL();

	//initialize Compute Shader for Slime Mold Particle Calculations 
	particleBehaviourShader_ = Shader("./res/ParticleBehaviour.cs", ShaderType::COMPUTE_SHADER);
	particleBehaviourProgram_ = ShaderProgram("ParticleBehaviourProgram");
	particleBehaviourProgram_.attachShader(particleBehaviourShader_.getShaderID());
	linkingSuccess = particleBehaviourProgram_.link();
	if(!linkingSuccess) {
		std::cerr << "Failed to link Particle Behaviour Shader Program. Exiting." << std::endl;
		return;
	}
	particleBehaviourProgram_.getUniformsFromGLSL();

	//------------------------------------------------------
	// Initialize Audio Recording and Processing
	//audioSystem_ = AudioSystem(params_.audioDevice);
	std::vector<std::string> availableAudioHardwareNames = audioSystem_.getAvailableHardwareDeviceNames();
	audioWindow_ = dynamic_cast<AudioWindow*>(ui_.getWindow("AudioWindow"));
	audioWindow_->addHardwareDeviceNames(availableAudioHardwareNames);

	//------------------------------------------------------
	// Initialize Preset System

	presetSystem_ = PresetSystem("./presets/", ".psf", &ui_);
	colorPresetSystem_ = ColorPresetSystem("./presets/", ".pcsf", &ui_);

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
	
	trailMapController_ = TrailMapController("./res/pictures/", ".png", 16, &ui_);	//Texture Unit 16 for Trail Mask Texture

	particleBehaviourProgram_.attachUniformBufferObject(universalShaderSettingsUBO_.getUniformBufferObjectID(), "UniversalShaderSettings", 0);
	particleBehaviourProgram_.attachUniformBufferObject(slimeSettingsUBO_.getUniformBufferObjectID(), "SlimeSettings", 1);
	particleBehaviourProgram_.attachUniformBufferObject(parameterSettingsUBO_.getUniformBufferObjectID(), "ParameterSettings", 4);

	trailDiffusionProgram_.attachUniformBufferObject(universalShaderSettingsUBO_.getUniformBufferObjectID(), "UniversalShaderSettings", 0);
	trailDiffusionProgram_.attachUniformBufferObject(trailDiffusionUBO_.getUniformBufferObjectID(), "TrailDiffusionSettings", 2);

	rasterizationPipeline_.attachUniformBufferObject(universalShaderSettingsUBO_.getUniformBufferObjectID(), "UniversalShaderSettings", 0);
	rasterizationPipeline_.attachUniformBufferObject(fragmentShaderSettingsUBO_.getUniformBufferObjectID(), "FragmentShaderSettings", 3);

	musicAnalysis_ = MusicAnalysis(uiState_);

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

		//------------------------------------------------------
		// Bloom Post-Processing (only if bloom is enabled)
		if(uiState_->fragmentShaderSettings.bloomEnabled) {
			bloomEffect_.applyBloom(texTrail_.getID(), &drawCanvas_, uiState_);
		}

		//------------------------------------------------------
		// Display Clearing
		window_.Clear(uiState_->clearColor.x, uiState_->clearColor.y, uiState_->clearColor.z, uiState_->clearColor.w);

		//------------------------------------------------------
		//OpenGL Draw Call
		glBindFramebuffer(GL_FRAMEBUFFER, 0); // Default framebuffer
		glViewport(0, 0, (int) ui_uss_.windowWidth, (int) ui_uss_.windowHeight);
		rasterizationPipeline_.use();

		// Bind main textures for fragment shader (these should always be bound)
		bloomEffect_.bindBloomTextures(texTrail_.getID(), texTrailNonDiffused_.getID(), newTexParticles_.getID(), oldTexParticles_.getID(), texCollisions_.getID());
		trailMapController_.bindToTextureUnit(16);
		drawCanvas_.draw();

		//------------------------------------------------------
		// ImGui Draw Call
		glBindFramebuffer(GL_FRAMEBUFFER, 0); // Default framebuffer to make sure
		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
		
		//------------------------------------------------------
		//process Key Presses from User
		window_.Update();	//TODO: badly named; RENAME!


		window_.setFullscreen(uiState_->fullscreen); //check if fullscreen mode changed in UI and set it
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

				texTrail_.resizeTexture((int)ui_uss_.textureWidth, (int)ui_uss_.textureHeight);
				texTrailNonDiffused_.resizeTexture((int)ui_uss_.textureWidth, (int)ui_uss_.textureHeight);
				newTexParticles_.resizeTexture((int)ui_uss_.textureWidth, (int)ui_uss_.textureHeight);
				oldTexParticles_.resizeTexture((int)ui_uss_.textureWidth, (int)ui_uss_.textureHeight);
				texCollisions_.resizeTexture((int)ui_uss_.textureWidth, (int)ui_uss_.textureHeight);

				bloomEffect_.resizeBloomTextures(ui_uss_.textureWidth, ui_uss_.textureHeight);
			}

			particleData_.createAndSend(uiState_->numParticles, ui_uss_.textureWidth, ui_uss_.textureHeight);
			universalShaderSettingsUBO_.updateUniformBufferObject(ui_uss_);
			uiState_->newCanvas = false;
		}

		//------------------------------------------------------
		// Handle Preset System

		//Saving Presets to File
		presetSystem_.handleUIRequests(&ui_);
		colorPresetSystem_.handleUIRequests(&ui_);
		trailMapController_.handleUIRequests(&ui_);

		//Auto Switching Presets
		presetSystem_.autoSwitchPresets(&ui_, timeInSeconds);
		colorPresetSystem_.autoSwitchPresets(&ui_, timeInSeconds);
		trailMapController_.autoSwitchPictures(&ui_, timeInSeconds);

		//Handle Audio Device Change TODO: Refactor to Audio Class!
		if(uiState_->selectAudioHardware) {
			audioSystem_.selectHardwareDevice(audioWindow_->getSelectedHardwareDevice());
			uiState_->selectAudioHardware = false;
		}

		//Exit Program when requested from UI
		if(uiState_->exitProgram) {
			if(!window_.setIsClosed(true)) {
				uiState_->exitProgram = false;
			}
		}
	}
}