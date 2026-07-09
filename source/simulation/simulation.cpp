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
	ApplicationState* uiState = ui->getState();	//TODO: whole ui used just for trailMapController, remove later!

	int textureWidth = uiState->universalShaderSettings.textureWidth;
	int textureHeight = uiState->universalShaderSettings.textureHeight;
	int workGroupDivider = uiState->workGroupDivider;
	
	//calculate new number of particles based on texture size, slime ratio and user input
	if(customParticleCount) {
		uiState->slimeRatio = uiState->numParticles / (float)(textureWidth * textureHeight);	//ensure number is multiples of workgroup size for compute shaders
	} else {
		uiState->numParticles = uiState->slimeRatio * textureWidth * textureHeight;
		uiState->numParticles = uiState->numParticles - uiState->numParticles % workGroupDivider;
	}
	
	//attach UBOs to compute shaders
	uboManager->attachUBOs({trailDiffusionProgram_.getProgramID(), particleBehaviourProgram_.getProgramID()});

	//------------------------------------------------------
	// Initialize Physarum Particles
	particleData_.createAndSend(uiState->numParticles, uiState->universalShaderSettings.textureWidth, uiState->universalShaderSettings.textureHeight);
}

void Simulation::simulateStep(ApplicationState* uiState) {
	int workGroupDivider = uiState->workGroupDivider;
	trailMapController_.bindToTextureUnit(5);	//because compute shaders use trailMask at texture unit 5

	trailDiffusionProgram_.dispatchCompute(uiState->universalShaderSettings.textureWidth / workGroupDivider, uiState->universalShaderSettings.textureHeight / workGroupDivider, 1);	//calculate new trail texture
	particleBehaviourProgram_.dispatchCompute(uiState->numParticles / 8, (GLuint)1, 1);	//move Slime Particles

	trailMapController_.bindToTextureUnit(16);	//move back to texture unit 16 for use in fragment shader 
}

void Simulation::setNewParticleParameters(ApplicationState* uiState) {
	particleData_.createAndSend(uiState->numParticles, uiState->universalShaderSettings.textureWidth, uiState->universalShaderSettings.textureHeight);
}