#ifndef MUSIC_ANALYSIS_H
#define MUSIC_ANALYSIS_H

#include "../ui/user_interface.h"

class MusicAnalysis {
public:

    MusicAnalysis() = default;
    MusicAnalysis(UIState* uiState);

    void analyzeMusic(std::vector<double>& spectrumDiff, double frameTime);

private:

    UIState* uiState_;

    float velocityReactionTimeConstant_ = 0.15f;
    float velocityBassReactionTarget_;
	float velocityBassReactionSmooth_;
};

#endif // MUSIC_ANALYSIS_H