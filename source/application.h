#ifndef APPLICATION_H
#define APPLICATION_H

#include <memory>

#include "color_preset_system.h"
#include "preset_system.h"
#include "window.h"
#include "./audio/audio_system.h"
#include "./audio/music_analysis.h"
#include "./graphics/renderer.h"
#include "./graphics/uniform_buffer_manager.h"
#include "./simulation/simulation.h"
#include "./simulation/trail_map_controller.h"
#include "./utility/parameter_parser.h"
#include "./ui/elements/audio_window.h"
#include "./ui/user_interface.h"

class Application {

public:
    Application(Parameters params);
    void run();

private:
    UIState* uiState_;
    UniversalShaderSettings& ui_uss_;	//just shortening the name as a temp solution

    Window window_;

    UserInterface ui_;

    UniformBufferManager ubo_manager_;

    Simulation simulation_;
    std::unique_ptr<Renderer> renderer_;

    AudioSystem audioSystem_;

    AudioWindow* audioWindow_;

    PresetSystem presetSystem_;
	ColorPresetSystem colorPresetSystem_;

    MusicAnalysis musicAnalysis_;

    Uint64 prevCounter_;
	Uint64 counterFrequency_; 
};

#endif // APPLICATION_H