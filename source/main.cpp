#include <iostream>
#include <iomanip>
#include "window.h"
#include "ParticleData.h"
#include "./graphics/Texture.h"
#include "./graphics/FrameBuffer.h"
#include "canvas.h"
#include "./ui/UserInterface.h"
#include "./ui/elements/AudioWindow.h"
#include "./audio/AudioRecording.h"
#include "./graphics/Shader.h"
#include "./graphics/ShaderProgram.h"
#include <SDL3/SDL_main.h>
#include <fftw3.h>
#include <SDL3/SDL_camera.h>
#include "./audio/AudioProcessor.h"
#include "PresetSystem.h"
#include "ColorPresetSystem.h"
#include "./graphics/UniformBufferObject.h"
#include <deque>
#include <algorithm>
#include "./graphics/Bloom.h"
#include "./audio/MusicAnalysis.h"

int main(int argc, char* argv[]) {

	// Default values for starting without command line arguments
	unsigned int textureWidth = 1600;
	unsigned int textureHeight = 896;
	unsigned int windowWidth = 1600;
	unsigned int windowHeight = 896;
	float slimeRatio = 0.15f;
	unsigned int numParticles = 300000;
	unsigned int workGroupDivider = 8;
	std::string deviceName = "";
	
	//helpMessage noch aktuell?
	std::string helpMessage = "Usage: " + std::string(argv[0]) + " [options]\n"
	+ "Options:\n"
	+ "  --help | -h          Show this help message\n"
	+ "  --width <pixels>     Set texture width (multiples of " + std::to_string(workGroupDivider) + ")\n"
	+ "  --height <pixels>    Set texture height (multiples of " + std::to_string(workGroupDivider) + ")\n"
	+ "  --particles <count>  Set number of particles (multiples of " + std::to_string(workGroupDivider) + ") (overwrites --slimeRatio <float>)\n"
	+ "  --slimeRatio <float> Set number of particles as a ratio of window area\n"
	+ "  --audioDevice <name> Set the audio device to use\n";
	
	//------------------------------------------------------		  
	// Parse command line arguments if any
	bool customResolution = false;
	bool customParticleCount = false;

	for (int i = 1; i < argc; i++) {
		if (strcmp(argv[i], "--width") == 0 && i + 1 < argc) {
			textureWidth = windowWidth = static_cast<unsigned int>(atoi(argv[i + 1]));
			customResolution = true;
			i++;
		}
		else if (strcmp(argv[i], "--height") == 0 && i + 1 < argc) {
			textureHeight = windowHeight = static_cast<unsigned int>(atoi(argv[i + 1]));
			customResolution = true;
			i++;
		}
		else if (strcmp(argv[i], "--particles") == 0 && i + 1 < argc) {
			numParticles = static_cast<unsigned int>(atoi(argv[i + 1]));
			customParticleCount = true;
			i++;
		}
		else if (strcmp(argv[i], "--slimeRatio") == 0 && i + 1 < argc) {
			slimeRatio = static_cast<float>(atof(argv[i + 1]));
			i++;
		}
		else if (strcmp(argv[i], "--audioDevice") == 0 && i + 1 < argc) {
			deviceName = std::string(argv[i + 1]);
			i++;
		}
		else if (strcmp(argv[i], "--help") == 0 || strcmp(argv[i], "-h") == 0) {
			std::cout << helpMessage << std::endl;
			return 0;
		} else {
			std::cout << helpMessage << std::endl;
  			return 0;
		}
	}

	//------------------------------------------------------
	// Initialize Window
	Window window((int)windowWidth, (int)windowHeight, "Physarum", customResolution);

	windowWidth = (unsigned int)window.getWindowWidth();
	windowHeight = (unsigned int)window.getWindowHeight();

	float scaling = window.getFractionalScalingFactor();

	textureWidth = windowWidth * scaling - ((unsigned int)(windowWidth * scaling) % workGroupDivider);		//make sure width is multiple of workgroup size
	textureHeight = windowHeight * scaling - ((unsigned int)(windowHeight * scaling) % workGroupDivider);	//make sure height is multiple of workgroup size

	if(!customParticleCount) {
		numParticles = slimeRatio * textureWidth * textureHeight;
		numParticles = numParticles - numParticles % workGroupDivider;
	} else {
		slimeRatio = numParticles / (float)(textureWidth * textureHeight);
	}

	Canvas drawCanvas;

	int maxVertexTextureUnits = 0;
	glGetIntegerv(GL_MAX_VERTEX_TEXTURE_IMAGE_UNITS, &maxVertexTextureUnits);
	std::cout << "Max Vertex Texture Image Units: " << maxVertexTextureUnits << std::endl;
	int maxFragmentTextureUnits = 0;
	glGetIntegerv(GL_MAX_TEXTURE_IMAGE_UNITS, &maxFragmentTextureUnits);
	std::cout << "Max Texture Image Units: " << maxFragmentTextureUnits << std::endl;
	int maxCombinedTextureUnits = 0;
	glGetIntegerv(GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS, &maxCombinedTextureUnits);
	std::cout << "Max Combined Texture Image Units: " << maxCombinedTextureUnits << std::endl;
	int maxImageUnits = 0;
	glGetIntegerv(GL_MAX_IMAGE_UNITS, &maxImageUnits);
	std::cout << "Max Image Units: " << maxImageUnits << std::endl;



	//------------------------------------------------------
	// Initialize User Interface
	UserInterface UserInterface(window.getWindow(), window.getGLContext());
	UIState& uiState = UserInterface.getState();
	uiState.universalShaderSettings.textureWidth = (int)textureWidth;
	uiState.universalShaderSettings.textureHeight = (int)textureHeight;
	uiState.universalShaderSettings.windowWidth = (int)textureWidth;
	uiState.universalShaderSettings.windowHeight = (int)textureHeight;
	uiState.newTextureWidth = (int)textureWidth;
	uiState.newTextureHeight = (int)textureHeight;
	uiState.numParticles = (int) (numParticles);
	uiState.newNumParticles = (int) (numParticles);
	uiState.slimeRatio = slimeRatio;

	//------------------------------------------------------
	//Initialize Textures and Texture Buffers
	Texture TexTrail((int)textureWidth, (int)textureHeight, Texture::TextureType::RGBA_FLOAT, 0);		//Texture Unit 0
	Texture TexTrailNonDiffused((int)textureWidth, (int)textureHeight, Texture::TextureType::RGBA_FLOAT, 1);	//Texture Unit 1
	Texture NewTexParticles((int)textureWidth, (int)textureHeight, Texture::TextureType::R_UINT, 2);		//Texture Unit 2
	Texture OldTexParticles((int)textureWidth, (int)textureHeight, Texture::TextureType::R_UINT, 3);		//Texture Unit 3
	Texture TexCollisions((int)textureWidth, (int)textureHeight, Texture::TextureType::RGBA_FLOAT, 4);		//Texture Unit 4

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
	Bloom bloomEffect(textureWidth, textureHeight, VertexShader.getShaderID());

	//------------------------------------------------------
	// Initialize Physarum Particles
	ParticleData ParticleData(numParticles, textureWidth, textureHeight);
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
	AudioRecording audioRecording(deviceName);
	std::vector<std::string> availableAudioHardwareNames = audioRecording.getAvailableHardwareDeviceNames();
	AudioWindow* audioWindow = dynamic_cast<AudioWindow*>(UserInterface.getWindow("AudioWindow"));

	for(unsigned int i = 0; i < availableAudioHardwareNames.size(); i++) {
		audioWindow->addHardwareDevice(availableAudioHardwareNames[i]);
	}

	//------------------------------------------------------
	// Initialize Preset System

	PresetSystem presetSystem("./presets/", ".psf");
	presetSystem.loadPresetNames(UserInterface);
	ColorPresetSystem colorPresetSystem("./presets/", ".pcsf");
	colorPresetSystem.loadPresetNames(UserInterface);

	UniformBufferObject UniversalShaderSettingsUBO(0);
	UniversalShaderSettingsUBO.bindUniformBufferObject(uiState.universalShaderSettings);
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

		uiState.universalShaderSettings.timeTicks = nowCounter;

		//------------------------------------------------------
		// setting Uniforms for later use in Draw Call
		UniversalShaderSettingsUBO.updateUniformBufferObject(uiState.universalShaderSettings);
		SlimeSettingsUBO.updateUniformBufferObject(uiState.slimeSettings);
		TrailDiffusionUBO.updateUniformBufferObject(uiState.trailDiffusionSettings);
		FragmentShaderSettingsUBO.updateUniformBufferObject(uiState.fragmentShaderSettings);
		ParameterSettingsUBO.updateUniformBufferObject(uiState.parameterSettings);

		//------------------------------------------------------
		// Compute Shader Passes for Simulation Steps

		TrailDiffusionProgram.dispatchCompute(textureWidth / workGroupDivider, textureHeight / workGroupDivider, 1);	//calculate new trail texture
		ParticleBehaviourProgram.dispatchCompute(numParticles / 8, (GLuint)1, 1);	//move Slime Particles

		//------------------------------------------------------
		// Audio Processing
		if(uiState.slimeSettings.reactToAudio) {
			audioRecording.computeSpectrum();
		}

		//give UI access to audio data; Why Here?!
		UserInterface.display(audioRecording.getAudioBuffer(), audioRecording.getSpectrum(), audioRecording.getSpectrumDiff(), audioRecording.getBufferSize(), audioRecording.hasNewSpectrumData());
		audioRecording.setHasNewSpectrumData(false);

		// Analyze the music data
		if(uiState.slimeSettings.reactToAudio) {
			musicAnalysis.analyzeMusic(audioRecording.getSpectrumDiff(), frameTime);
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
		glViewport(0, 0, (int) uiState.universalShaderSettings.windowWidth, (int) uiState.universalShaderSettings.windowHeight);
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
		uiState.universalShaderSettings.windowWidth = window.getWindowWidth();		//synchronize window size
		uiState.universalShaderSettings.windowHeight = window.getWindowHeight();

		//------------------------------------------------------
		// Handle User Interface Changes that affect Simulation State
		if(uiState.newCanvas) {
			std::cout << "Creating new Canvas with " << numParticles << " particles and size " << textureWidth << "x" << textureHeight << std::endl;
			if(uiState.newTextureWidth != uiState.universalShaderSettings.textureWidth || uiState.newTextureHeight != uiState.universalShaderSettings.textureHeight) {
				// Resize Textures and Framebuffers
				textureWidth = (unsigned int)uiState.newTextureWidth;
				textureHeight = (unsigned int)uiState.newTextureHeight;
				uiState.universalShaderSettings.textureWidth = (int)textureWidth;
				uiState.universalShaderSettings.textureHeight = (int)textureHeight;

				TexTrail.resizeTexture((int)textureWidth, (int)textureHeight);
				TexTrailNonDiffused.resizeTexture((int)textureWidth, (int)textureHeight);
				NewTexParticles.resizeTexture((int)textureWidth, (int)textureHeight);
				OldTexParticles.resizeTexture((int)textureWidth, (int)textureHeight);
				TexCollisions.resizeTexture((int)textureWidth, (int)textureHeight);

				bloomEffect.resizeBloomTextures(textureWidth, textureHeight);
			}

			numParticles = (unsigned int)uiState.numParticles;

			ParticleData.recreateAndSend(numParticles, textureWidth, textureHeight);
			UniversalShaderSettingsUBO.updateUniformBufferObject(uiState.universalShaderSettings);
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
			audioRecording.selectHardwareDevice(audioWindow->getSelectedHardwareDevice());
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