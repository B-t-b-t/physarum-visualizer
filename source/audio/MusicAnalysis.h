#ifndef MUSIC_ANALYSIS_H
#define MUSIC_ANALYSIS_H

#include "../ui/UserInterface.h"

class MusicAnalysis {
public:

    MusicAnalysis(UIState& uiState);

    void analyzeMusic(std::vector<double>& spectrumDiff, double frameTime);

private:

    UIState& uiState_;

    const float velocityReactionTimeConstant_ = 0.15f;
    float velocityBassReactionTarget_;
	float velocityBassReactionSmooth_;
};

#endif // MUSIC_ANALYSIS_H