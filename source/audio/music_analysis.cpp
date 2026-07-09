#include "music_analysis.h"

MusicAnalysis::MusicAnalysis(ApplicationState* appState) : appState_(appState) {
    velocityBassReactionTarget_ = appState_->slimeSettings.velocityBassReaction;
	velocityBassReactionSmooth_ = appState_->slimeSettings.velocityBassReaction;
}

void MusicAnalysis::analyzeMusic(std::vector<double>& spectrumDiff, double frameTime) {
    //------------------------------------------------------
    // Beat Analysis for Audio Reaction Mode
    int subBass = 0;
    int bass = 0;
    int lowMidRange = 0;
    int midRange = 0;
    int upperMidRange = 0;
    int presence = 0;
    int brilliance = 0;

    if(spectrumDiff.size() > 0) {
        for(std::vector<double>::size_type i = 0; i < spectrumDiff.size(); i++) {
            if(i >= 2 && i <= 5) {
                //Sub Bass 20-60 Hz
                double value = spectrumDiff[i] > 0 ? spectrumDiff[i] : 0.0;
                subBass += value;
                appState_->subBassValue = subBass;
            } else if(i >= 6 && i <= 21) {
                //Bass 60-250 Hz
                double value = spectrumDiff[i] > 0 ? spectrumDiff[i] : 0.0;
                bass += value;
                appState_->bassValue = bass;
            } else if(i >= 22 && i <= 42) {
                //Low Mid Range 250-500 Hz
                double value = spectrumDiff[i] > 0 ? spectrumDiff[i] : 0.0;
                lowMidRange += value;
                appState_->lowMidRangeValue = lowMidRange;
            } else if(i >= 43 && i <= 170) {
                //Mid Range 500-2000 Hz
                double value = spectrumDiff[i] > 0 ? spectrumDiff[i] : 0.0;
                midRange += value;
                appState_->midRangeValue = midRange;
            } else if(i >= 171 && i <= 341) {
                //Upper Mid Range 2000-4000 Hz
                double value = spectrumDiff[i] > 0 ? spectrumDiff[i] : 0.0;
                upperMidRange += value;
                appState_->upperMidRangeValue = upperMidRange;
            } else if(i >= 342 && i <= 512) {
                //Presence 4000-6000 Hz
                double value = spectrumDiff[i] > 0 ? spectrumDiff[i] : 0.0;
                presence += value;
                appState_->presenceValue = presence;
            } else if(i >= 513) {
                //Brilliance 6000-20000 Hz
                double value = spectrumDiff[i] > 0 ? spectrumDiff[i] : 0.0;
                brilliance += value;
                appState_->brillianceValue = brilliance;
            }
        }

        if(subBass > 50) {appState_->subBassDetected = true;} else {appState_->subBassDetected = false;}
        if(bass > 100) {appState_->bassDetected = true;} else {appState_->bassDetected = false;}
        if(lowMidRange > 100) {appState_->lowMidRangeDetected = true;} else {appState_->lowMidRangeDetected = false;}
        if(midRange > 100) {appState_->midRangeDetected = true;} else {appState_->midRangeDetected = false;}
        if(upperMidRange > 100) {appState_->upperMidRangeDetected = true;} else {appState_->upperMidRangeDetected = false;}
        if(presence > 100) {appState_->presenceDetected = true;} else {appState_->presenceDetected = false;}
        if(brilliance > 100) {appState_->brillianceDetected = true;} else {appState_->brillianceDetected = false;}

        if(appState_->bassDetected && appState_->bloomAudioReaction) {
            velocityBassReactionTarget_ += 0.5f;
            appState_->fragmentShaderSettings.bloomBassReaction = appState_->bloomBassReactionIntensity;
            appState_->slimeSettings.angleBassReaction = 0;
        } else {
            appState_->fragmentShaderSettings.bloomBassReaction = 0.0f;
        }
    }
    

    // Smooth value using exponential smoothing (time-constant based)
    float alpha = 0.0f;
    if(velocityReactionTimeConstant_ > 0.0f) {
        alpha = 1.0f - std::exp(-1.0f / (velocityReactionTimeConstant_ * 60.0f));
    } else {
        alpha = 1.0f;
    }
    velocityBassReactionSmooth_ += (velocityBassReactionTarget_ - velocityBassReactionSmooth_) * alpha;

    // Exponential decay of the target so impulses fade naturally.
    // Use appState.beatDivide as a decay rate proxy (tune as needed)
    float decayRate = std::max(0.0f, appState_->beatDivide);
    velocityBassReactionTarget_ *= std::expf(-decayRate * (float)frameTime);

    // clamp
    if(velocityBassReactionSmooth_ < 0.0f) velocityBassReactionSmooth_ = 0.0f;
    if(velocityBassReactionTarget_ < 0.0f) velocityBassReactionTarget_ = 0.0f;

    // write the smoothed value back into appState so UBO uses it
    appState_->slimeSettings.velocityBassReaction = velocityBassReactionSmooth_;
}