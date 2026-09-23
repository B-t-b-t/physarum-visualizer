#ifndef APPLICATION_H
#define APPLICATION_H

#include "input_handler.h"
#include "preset_system.h"
#include "window.h"
#include "./audio/audio_system.h"
#include "./audio/music_analysis.h"
#include "./graphics/renderer.h"
#include "./graphics/uniform_buffer_manager.h"
#include "./simulation/simulation.h"
#include "./ui/windows/audio_window.h"
#include "./ui/user_interface.h"

struct Parameters;	//forward declaration of Parameters struct

class Application {

public:
    Application(Parameters params);
    void run();

private:
    ApplicationState* appState_;

    Window window_;

    InputHandler inputHandler_;

    UserInterface ui_;

    UniformBufferManager ubo_manager_;

    Simulation simulation_;
    std::unique_ptr<Renderer> renderer_;

    AudioSystem audioSystem_;

    AudioWindow* audioWindow_;

    PresetSystem<BehaviorPreset> presetSystem_;
	PresetSystem<ColorPreset> colorPresetSystem_;

    MusicAnalysis musicAnalysis_;

    Uint64 prevCounter_;
	Uint64 counterFrequency_; 
};

#endif // APPLICATION_H