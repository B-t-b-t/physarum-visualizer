#ifndef SIMULATION_H
#define SIMULATION_H

#include "trail_map_controller.h"
#include "../particle_data.h"
#include "../graphics/shader.h"
#include "../graphics/shader_program.h"
#include "../graphics/uniform_buffer_manager.h"
#include "../ui/ui_state.h"
#include "../ui/user_interface.h"

class Simulation {

public:

    Simulation(UniformBufferManager* uboManager, UserInterface* ui, bool customParticleCount);

    void simulateStep(UIState* uiState);
    void setNewParticleParameters(UIState* uiState);
    TrailMapController* getTrailMapController() { return &trailMapController_; }

private:
    ParticleData particleData_;

    Shader trailDiffusionShader_;
	ShaderProgram trailDiffusionProgram_;
    Shader particleBehaviourShader_;
	ShaderProgram particleBehaviourProgram_;

    TrailMapController trailMapController_;
};

#endif // SIMULATION_H