#include <algorithm>
#include <deque>
#include <iomanip>
#include <iostream>

#include <fftw3.h>
#include <SDL3/SDL_camera.h>
#include <SDL3/SDL_main.h>

#include "canvas.h"
#include "color_preset_system.h"
#include "particle_data.h"
#include "preset_system.h"
#include "window.h"
#include "./audio/audio_system.h"
#include "./audio/audio_processor.h"
#include "./audio/music_analysis.h"
#include "./graphics/bloom.h"
#include "./graphics/framebuffer.h"
#include "./graphics/shader.h"
#include "./graphics/shader_program.h"
#include "./graphics/texture.h"
#include "./graphics/uniform_buffer_object.h"
#include "./ui/elements/audio_window.h"
#include "./ui/user_interface.h"
#include "./utility/parameter_parser.h"

int main(int argc, char* argv[]) {

	// Default values for starting without command line arguments
	int workGroupDivider = 8;
	std::string deviceName = "";
	
	//Parse command line arguments
	Parameters params;

	int parseSuccess = parseParameters(argc, argv, params, workGroupDivider);

	if(parseSuccess == -1) {
		printHelpMessage(argv, workGroupDivider);
		return 0;
	}

	deviceName = params.audioDevice;
	

	//------------------------------------------------------
	// Initialize Window
	Window window((int)params.width, (int)params.height, "Physarum", params.customResolution);
	
	//------------------------------------------------------
	// Initialize User Interface
	UserInterface UserInterface(window.getWindow(), window.getGLContext());
	UIState& uiState = UserInterface.getState();

	auto& ui_uss = uiState.universalShaderSettings;	//just shortening the name as a temp solution
	
	ui_uss.windowWidth = window.getWindowWidth();
	ui_uss.windowHeight = window.getWindowHeight();
	uiState.numParticles = params.numParticles;
	uiState.newNumParticles = params.numParticles;
	uiState.slimeRatio = params.slimeRatio;

	float scaling = window.getFractionalScalingFactor();

	ui_uss.textureWidth = (int)(ui_uss.windowWidth * scaling - ((int)(ui_uss.windowWidth * scaling) % workGroupDivider));		//make sure width is multiple of workgroup size
	ui_uss.textureHeight = (int)(ui_uss.windowHeight * scaling - ((int)(ui_uss.windowHeight * scaling) % workGroupDivider));	//make sure height is multiple of workgroup size
	uiState.newTextureWidth = ui_uss.textureWidth;
	uiState.newTextureHeight = ui_uss.textureHeight;

	if(params.customParticleCount) {
		uiState.slimeRatio = uiState.numParticles / (float)(ui_uss.textureWidth * ui_uss.textureHeight);
	} else {
		uiState.numParticles = uiState.slimeRatio * ui_uss.textureWidth * ui_uss.textureHeight;
		uiState.numParticles = uiState.numParticles - uiState.numParticles % workGroupDivider;	//ensure multiple of workgroup size
	}

	Canvas drawCanvas;

	//------------------------------------------------------
	//Initialize Textures and Texture Buffers
	Texture TexTrail((int)ui_uss.textureWidth, (int)ui_uss.textureHeight, Texture::TextureType::RGBA_FLOAT, 0);		//Texture Unit 0
	Texture TexTrailNonDiffused((int)ui_uss.textureWidth, (int)ui_uss.textureHeight, Texture::TextureType::RGBA_FLOAT, 1);	//Texture Unit 1
	Texture NewTexParticles((int)ui_uss.textureWidth, (int)ui_uss.textureHeight, Texture::TextureType::R_UINT, 2);		//Texture Unit 2
	Texture OldTexParticles((int)ui_uss.textureWidth, (int)ui_uss.textureHeight, Texture::TextureType::R_UINT, 3);		//Texture Unit 3
	Texture TexCollisions((int)ui_uss.textureWidth, (int)ui_uss.textureHeight, Texture::TextureType::RGBA_FLOAT, 4);		//Texture Unit 4

	//------------------------------------------------------
	//initialize Rasterizer Pipeline for Canvas
	Shader VertexShader("./res/vertex.vs", ShaderType::VERTEX_SHADER);
	Shader FragmentShader("./res/fragment.fs", ShaderType::FRAGMENT_SHADER);

	ShaderProgram RasterizationPipeline("RasterizationPipeline");
	RasterizationPipeline.attachShader(VertexShader.getShaderID());
	RasterizationPipeline.attachShader(FragmentShader.getShaderID());
	RasterizationPipeline.link();
	RasterizationPipeline.getUniformsFromGLSL();

	// Initialize Bloom Effect
	Bloom bloomEffect(ui_uss.textureWidth, ui_uss.textureHeight, VertexShader.getShaderID());

	//------------------------------------------------------
	// Initialize Physarum Particles
	ParticleData ParticleData(uiState.numParticles, ui_uss.textureWidth, ui_uss.textureHeight);
	ParticleData.createAndSend();

	//------------------------------------------------------
	// Initialize Compute Shaders

	//initialize Compute Shader for Slime Mold Trail Texture
	Shader TrailDiffusionShader("./res/TrailDiffusion.cs", ShaderType::COMPUTE_SHADER);
	ShaderProgram TrailDiffusionProgram("TrailDiffusionProgram");
	TrailDiffusionProgram.attachShader(TrailDiffusionShader.getShaderID());
	TrailDiffusionProgram.link();
	TrailDiffusionProgram.getUniformsFromGLSL();

	//initialize Compute Shader for Slime Mold Particle Calculations 
	Shader ParticleBehaviourShader("./res/ParticleBehaviour.cs", ShaderType::COMPUTE_SHADER);
	ShaderProgram ParticleBehaviourProgram("ParticleBehaviourProgram");
	ParticleBehaviourProgram.attachShader(ParticleBehaviourShader.getShaderID());
	ParticleBehaviourProgram.link();
	ParticleBehaviourProgram.getUniformsFromGLSL();

	//------------------------------------------------------
	// Initialize Audio Recording and Processing
	AudioSystem audioSystem(deviceName);
	std::vector<std::string> availableAudioHardwareNames = audioSystem.getAvailableHardwareDeviceNames();
	AudioWindow* audioWindow = dynamic_cast<AudioWindow*>(UserInterface.getWindow("AudioWindow"));
	audioWindow->addHardwareDeviceNames(availableAudioHardwareNames);

	//------------------------------------------------------
	// Initialize Preset System

	PresetSystem presetSystem("./presets/", ".psf");
	presetSystem.loadPresetNames(UserInterface);
	ColorPresetSystem colorPresetSystem("./presets/", ".pcsf");
	colorPresetSystem.loadPresetNames(UserInterface);

	UniformBufferObject UniversalShaderSettingsUBO(0);
	UniversalShaderSettingsUBO.bindUniformBufferObject(ui_uss);
	UniformBufferObject SlimeSettingsUBO(1);
	SlimeSettingsUBO.bindUniformBufferObject(uiState.slimeSettings);
	UniformBufferObject TrailDiffusionUBO(2);
	TrailDiffusionUBO.bindUniformBufferObject(uiState.trailDiffusionSettings);
	UniformBufferObject FragmentShaderSettingsUBO(3);
	FragmentShaderSettingsUBO.bindUniformBufferObject(uiState.fragmentShaderSettings);
	UniformBufferObject ParameterSettingsUBO(4);
	ParameterSettingsUBO.bindUniformBufferObject(uiState.parameterSettings);
	


	ParticleBehaviourProgram.attachUniformBufferObject(UniversalShaderSettingsUBO.getUniformBufferObjectID(), "UniversalShaderSettings", 0);
	ParticleBehaviourProgram.attachUniformBufferObject(SlimeSettingsUBO.getUniformBufferObjectID(), "SlimeSettings", 1);
	ParticleBehaviourProgram.attachUniformBufferObject(ParameterSettingsUBO.getUniformBufferObjectID(), "ParameterSettings", 4);

	TrailDiffusionProgram.attachUniformBufferObject(UniversalShaderSettingsUBO.getUniformBufferObjectID(), "UniversalShaderSettings", 0);
	TrailDiffusionProgram.attachUniformBufferObject(TrailDiffusionUBO.getUniformBufferObjectID(), "TrailDiffusionSettings", 2);

	RasterizationPipeline.attachUniformBufferObject(UniversalShaderSettingsUBO.getUniformBufferObjectID(), "UniversalShaderSettings", 0);
	RasterizationPipeline.attachUniformBufferObject(FragmentShaderSettingsUBO.getUniformBufferObjectID(), "FragmentShaderSettings", 3);

	MusicAnalysis musicAnalysis(uiState);

	Uint64 prevCounter = SDL_GetPerformanceCounter();
	Uint64 counterFrequency = SDL_GetPerformanceFrequency(); //SDL Timer Frequency for Audio Beat Analysis and Auto Preset Switching

	//======================================================================
	// Main Loop
	//======================================================================
	while (!window.IsClosed()) {


		Uint64 nowCounter = SDL_GetPerformanceCounter();
		Uint64 timeInSeconds = nowCounter / counterFrequency; //SDL Timer in Seconds for Audio Beat Analysis and Auto Preset Switching
		double frameTime = double(nowCounter - prevCounter) / double(counterFrequency);
		prevCounter = nowCounter;

		ui_uss.timeTicks = nowCounter;

		//------------------------------------------------------
		// setting Uniforms for later use in Draw Call
		UniversalShaderSettingsUBO.updateUniformBufferObject(ui_uss);
		SlimeSettingsUBO.updateUniformBufferObject(uiState.slimeSettings);
		TrailDiffusionUBO.updateUniformBufferObject(uiState.trailDiffusionSettings);
		FragmentShaderSettingsUBO.updateUniformBufferObject(uiState.fragmentShaderSettings);
		ParameterSettingsUBO.updateUniformBufferObject(uiState.parameterSettings);

		//------------------------------------------------------
		// Compute Shader Passes for Simulation Steps

		TrailDiffusionProgram.dispatchCompute(ui_uss.textureWidth / workGroupDivider, ui_uss.textureHeight / workGroupDivider, 1);	//calculate new trail texture
		ParticleBehaviourProgram.dispatchCompute(uiState.numParticles / 8, (GLuint)1, 1);	//move Slime Particles

		//------------------------------------------------------
		// Audio Processing
		if(uiState.slimeSettings.reactToAudio) {
			audioSystem.computeSpectrum();
		}

		//give UI access to audio data; Why Here?!
		UserInterface.display(audioSystem.getAudioBuffer(), audioSystem.getSpectrum(), audioSystem.getSpectrumDiff(), audioSystem.getBufferSize(), audioSystem.hasNewSpectrumData());
		audioSystem.setHasNewSpectrumData(false);

		// Analyze the music data
		if(uiState.slimeSettings.reactToAudio) {
			musicAnalysis.analyzeMusic(audioSystem.getSpectrumDiff(), frameTime);
		}

		//------------------------------------------------------
		// Bloom Post-Processing (only if bloom is enabled)
		if(uiState.fragmentShaderSettings.bloomEnabled) {
			bloomEffect.applyBloom(TexTrail.getID(), drawCanvas, uiState);
		}

		//------------------------------------------------------
		// Display Clearing
		window.Clear(uiState.clearColor.x, uiState.clearColor.y, uiState.clearColor.z, uiState.clearColor.w);

		//------------------------------------------------------
		//OpenGL Draw Call
		glBindFramebuffer(GL_FRAMEBUFFER, 0); // Default framebuffer
		glViewport(0, 0, (int) ui_uss.windowWidth, (int) ui_uss.windowHeight);
		RasterizationPipeline.use();

		// Bind main textures for fragment shader (these should always be bound)
		bloomEffect.bindBloomTextures(TexTrail.getID(), TexTrailNonDiffused.getID(), NewTexParticles.getID(), OldTexParticles.getID(), TexCollisions.getID());
		drawCanvas.draw();

		//------------------------------------------------------
		// ImGui Draw Call
		glBindFramebuffer(GL_FRAMEBUFFER, 0); // Default framebuffer to make sure
		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
		
		//------------------------------------------------------
		//process Key Presses from User
		window.Update();	//TODO: badly named; RENAME!


		window.setFullscreen(uiState.fullscreen); //check if fullscreen mode changed in UI and set it
		if(window.getExitLock()) { 				//if exit lock is active go to fullscreen
			uiState.fullscreen = true;
		}
		ui_uss.windowWidth = window.getWindowWidth();		//synchronize window size
		ui_uss.windowHeight = window.getWindowHeight();

		//------------------------------------------------------
		// Handle User Interface Changes that affect Simulation State
		if(uiState.newCanvas) {
			std::cout << "Creating new Canvas with " << uiState.numParticles << " particles and size " << ui_uss.textureWidth << "x" << ui_uss.textureHeight << std::endl;
			if(uiState.newTextureWidth != ui_uss.textureWidth || uiState.newTextureHeight != ui_uss.textureHeight) {
				// Resize Textures and Framebuffers
				ui_uss.textureWidth = uiState.newTextureWidth;
				ui_uss.textureHeight = uiState.newTextureHeight;

				TexTrail.resizeTexture((int)ui_uss.textureWidth, (int)ui_uss.textureHeight);
				TexTrailNonDiffused.resizeTexture((int)ui_uss.textureWidth, (int)ui_uss.textureHeight);
				NewTexParticles.resizeTexture((int)ui_uss.textureWidth, (int)ui_uss.textureHeight);
				OldTexParticles.resizeTexture((int)ui_uss.textureWidth, (int)ui_uss.textureHeight);
				TexCollisions.resizeTexture((int)ui_uss.textureWidth, (int)ui_uss.textureHeight);

				bloomEffect.resizeBloomTextures(ui_uss.textureWidth, ui_uss.textureHeight);
			}

			ParticleData.recreateAndSend(uiState.numParticles, ui_uss.textureWidth, ui_uss.textureHeight);
			UniversalShaderSettingsUBO.updateUniformBufferObject(ui_uss);
			uiState.newCanvas = false;
		}

		//------------------------------------------------------
		// Handle Preset System

		//Saving Presets to File
		presetSystem.handleUIRequests(UserInterface);
		colorPresetSystem.handleUIRequests(UserInterface);

		//Auto Switching Presets
		presetSystem.autoSwitchPresets(UserInterface, timeInSeconds);
		colorPresetSystem.autoSwitchPresets(UserInterface, timeInSeconds);

		//Handle Audio Device Change TODO: Refactor to Audio Class!
		if(uiState.selectAudioHardware) {
			audioSystem.selectHardwareDevice(audioWindow->getSelectedHardwareDevice());
			uiState.selectAudioHardware = false;
		}

		//Exit Program when requested from UI
		if(uiState.exitProgram) {
			if(!window.setIsClosed(true)) {
				uiState.exitProgram = false;
			}
		}
	}

	window.~Window(); //call destructor and pause to view console seperatly after closing the drawing window
	//system("pause");

	return 0;
}