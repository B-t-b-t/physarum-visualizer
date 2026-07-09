#ifndef SIMULATION_H
#define SIMULATION_H

#include "trail_map_controller.h"
#include "../particle_data.h"
#include "../graphics/shader.h"
#include "../graphics/shader_program.h"
#include "../graphics/uniform_buffer_manager.h"
#include "../application_state.h"
#include "../ui/user_interface.h"
#include "../utility/observer.h"

class Simulation : public Observer {

public:

    Simulation(UniformBufferManager* uboManager, UserInterface* ui, bool customParticleCount);

    void simulateStep();
    void updateParticleParameters();
    TrailMapController* getTrailMapController() { return &trailMapController_; }

    void onNotify(const Event event) override;

private:
    ParticleData particleData_;

    Shader trailDiffusionShader_;
	ShaderProgram trailDiffusionProgram_;
    Shader particleBehaviourShader_;
	ShaderProgram particleBehaviourProgram_;

    TrailMapController trailMapController_;

    ApplicationState* appState_;
};

#endif // SIMULATION_H