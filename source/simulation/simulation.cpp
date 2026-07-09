#include "simulation.h"

Simulation::Simulation(UniformBufferManager* uboManager, UserInterface* ui, bool customParticleCount)
 : 	trailDiffusionShader_{Shader("./res/TrailDiffusion.cs", ShaderType::COMPUTE_SHADER)},
	trailDiffusionProgram_{ShaderProgram("TrailDiffusionProgram", {&trailDiffusionShader_})},
	particleBehaviourShader_{Shader("./res/ParticleBehaviour.cs", ShaderType::COMPUTE_SHADER)},
	particleBehaviourProgram_{ShaderProgram("ParticleBehaviourProgram", {&particleBehaviourShader_})},
	trailMapController_{TrailMapController("./res/pictures/", ".png", 16, ui)}	//Texture Unit 16 for Trail Mask Texture
{
	//------------------------------------------------------
	// Calculate new simulation parameters based on window properties or user input
	appState_ = ui->getState();

	int textureWidth = appState_->universalShaderSettings.textureWidth;
	int textureHeight = appState_->universalShaderSettings.textureHeight;
	int workGroupDivider = appState_->workGroupDivider;
	
	//calculate new number of particles based on texture size, slime ratio and user input
	if(customParticleCount) {
		appState_->slimeRatio = appState_->numParticles / (float)(textureWidth * textureHeight);	//ensure number is multiples of workgroup size for compute shaders
	} else {
		appState_->numParticles = appState_->slimeRatio * textureWidth * textureHeight;
		appState_->numParticles = appState_->numParticles - appState_->numParticles % workGroupDivider;
	}
	
	//attach UBOs to compute shaders
	uboManager->attachUBOs({trailDiffusionProgram_.getProgramID(), particleBehaviourProgram_.getProgramID()});

	//------------------------------------------------------
	// Initialize Physarum Particles
	particleData_.createAndSend(appState_->numParticles, appState_->universalShaderSettings.textureWidth, appState_->universalShaderSettings.textureHeight);
}

void Simulation::simulateStep() {
	int workGroupDivider = appState_->workGroupDivider;
	trailMapController_.bindToTextureUnit(5);	//because compute shaders use trailMask at texture unit 5

	trailDiffusionProgram_.dispatchCompute(appState_->universalShaderSettings.textureWidth / workGroupDivider, appState_->universalShaderSettings.textureHeight / workGroupDivider, 1);	//calculate new trail texture
	particleBehaviourProgram_.dispatchCompute(appState_->numParticles / 8, (GLuint)1, 1);	//move Slime Particles

	trailMapController_.bindToTextureUnit(16);	//move back to texture unit 16 for use in fragment shader 
}

void Simulation::updateParticleParameters() {
	particleData_.createAndSend(appState_->numParticles, appState_->universalShaderSettings.textureWidth, appState_->universalShaderSettings.textureHeight);
}

void Simulation::onNotify(const Event event) {
	switch(event) {
		case Event::NEW_CANVAS:
			updateParticleParameters();
			break;
		default:
			break;
	}
}