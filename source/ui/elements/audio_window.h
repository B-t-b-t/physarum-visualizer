#ifndef AUDIO_WINDOW_H
#define AUDIO_WINDOW_H

#include "base_window.h"
#include "../user_interface.h"
#include "../../application_state.h"

class AudioWindow : public BaseWindow {

public:
    AudioWindow();
    void render(ApplicationState* appState) override;

private:

	std::vector<double> heatmapData_;
	std::vector<double> heatMapChange_;
	int heatMapIndex_ = 0;
};

#endif // AUDIO_WINDOW_H