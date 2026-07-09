#ifndef SIMULATION_H
#define SIMULATION_H

#include "../particle_data.h"
#include "../graphics/shader.h"
#include "../graphics/shader_program.h"
#include "../graphics/uniform_buffer_manager.h"
#include "../ui/ui_state.h"

class Simulation {

public:

    Simulation(UniformBufferManager* uboManager, UIState* uiState, bool customParticleCount);

    void simulateStep(UIState* uiState);
    void setNewParticleParameters(UIState* uiState);

private:
    ParticleData particleData_;

    Shader trailDiffusionShader_;
	ShaderProgram trailDiffusionProgram_;
    Shader particleBehaviourShader_;
	ShaderProgram particleBehaviourProgram_;

};

#endif // SIMULATION_H