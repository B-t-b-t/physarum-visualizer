#include "simulation.h"

Simulation::Simulation(UniformBufferManager* uboManager, UIState* uiState, bool customParticleCount)
 : 	trailDiffusionShader_{Shader("./res/TrailDiffusion.cs", ShaderType::COMPUTE_SHADER)},
	trailDiffusionProgram_{ShaderProgram("TrailDiffusionProgram", {&trailDiffusionShader_})},
	particleBehaviourShader_{Shader("./res/ParticleBehaviour.cs", ShaderType::COMPUTE_SHADER)},
	particleBehaviourProgram_{ShaderProgram("ParticleBehaviourProgram", {&particleBehaviourShader_})}
{
	//------------------------------------------------------
	// Calculate new simulation parameters based on window properties or user input
	int windowWidth = uiState->universalShaderSettings.windowWidth;
	int windowHeight = uiState->universalShaderSettings.windowHeight;
	float scaling = uiState->fractionalScalingFactor;
	int workGroupDivider = uiState->workGroupDivider;

	//calculate texture sizes based on window size and fractional scaling
	int textureWidth = (int)(windowWidth * scaling - ((int)(windowWidth * scaling) % workGroupDivider));		//make sure width is multiples of workgroup size for compute shaders
	int textureHeight = (int)(windowHeight * scaling - ((int)(windowHeight * scaling) % workGroupDivider));
	
	//calculate new number of particles based on texture size, slime ratio and 
	if(customParticleCount) {
		uiState->slimeRatio = uiState->numParticles / (float)(textureWidth * textureHeight);	//ensure number is multiples of workgroup size for compute shaders
	} else {
		uiState->numParticles = uiState->slimeRatio * textureWidth * textureHeight;
		uiState->numParticles = uiState->numParticles - uiState->numParticles % workGroupDivider;
	}
	
	//write back to program state
	uiState->universalShaderSettings.textureWidth = textureWidth;
	uiState->universalShaderSettings.textureHeight = textureHeight;
	uiState->newTextureWidth = textureWidth;
	uiState->newTextureHeight = textureHeight;
	
	//attach UBOs to compute shaders
	uboManager->attachUBOs({trailDiffusionProgram_.getProgramID(), particleBehaviourProgram_.getProgramID()});

	//------------------------------------------------------
	// Initialize Physarum Particles
	particleData_.createAndSend(uiState->numParticles, uiState->universalShaderSettings.textureWidth, uiState->universalShaderSettings.textureHeight);
}

void Simulation::simulateStep(UIState* uiState) {
	int workGroupDivider = uiState->workGroupDivider;

	trailDiffusionProgram_.dispatchCompute(uiState->universalShaderSettings.textureWidth / workGroupDivider, uiState->universalShaderSettings.textureHeight / workGroupDivider, 1);	//calculate new trail texture
	particleBehaviourProgram_.dispatchCompute(uiState->numParticles / 8, (GLuint)1, 1);	//move Slime Particles
}

void Simulation::setNewParticleParameters(UIState* uiState) {
	particleData_.createAndSend(uiState->numParticles, uiState->universalShaderSettings.textureWidth, uiState->universalShaderSettings.textureHeight);
}