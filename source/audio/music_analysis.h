#ifndef MUSIC_ANALYSIS_H
#define MUSIC_ANALYSIS_H

#include "../ui/user_interface.h"

class MusicAnalysis {
public:

    MusicAnalysis() = default;
    MusicAnalysis(ApplicationState* appState);

    void analyzeMusic(std::vector<double>& spectrumDiff, double frameTime);

private:

    ApplicationState* appState_;

    float velocityReactionTimeConstant_ = 0.15f;
    float velocityBassReactionTarget_;
	float velocityBassReactionSmooth_;
};

#endif // MUSIC_ANALYSIS_H