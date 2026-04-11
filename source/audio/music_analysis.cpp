#include "music_analysis.h"

MusicAnalysis::MusicAnalysis(UIState& uiState) : uiState_(uiState) {
    velocityBassReactionTarget_ = uiState_.slimeSettings.velocityBassReaction;
	velocityBassReactionSmooth_ = uiState_.slimeSettings.velocityBassReaction;
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
                uiState_.subBassValue = subBass;
            } else if(i >= 6 && i <= 21) {
                //Bass 60-250 Hz
                double value = spectrumDiff[i] > 0 ? spectrumDiff[i] : 0.0;
                bass += value;
                uiState_.bassValue = bass;
            } else if(i >= 22 && i <= 42) {
                //Low Mid Range 250-500 Hz
                double value = spectrumDiff[i] > 0 ? spectrumDiff[i] : 0.0;
                lowMidRange += value;
                uiState_.lowMidRangeValue = lowMidRange;
            } else if(i >= 43 && i <= 170) {
                //Mid Range 500-2000 Hz
                double value = spectrumDiff[i] > 0 ? spectrumDiff[i] : 0.0;
                midRange += value;
                uiState_.midRangeValue = midRange;
            } else if(i >= 171 && i <= 341) {
                //Upper Mid Range 2000-4000 Hz
                double value = spectrumDiff[i] > 0 ? spectrumDiff[i] : 0.0;
                upperMidRange += value;
                uiState_.upperMidRangeValue = upperMidRange;
            } else if(i >= 342 && i <= 512) {
                //Presence 4000-6000 Hz
                double value = spectrumDiff[i] > 0 ? spectrumDiff[i] : 0.0;
                presence += value;
                uiState_.presenceValue = presence;
            } else if(i >= 513) {
                //Brilliance 6000-20000 Hz
                double value = spectrumDiff[i] > 0 ? spectrumDiff[i] : 0.0;
                brilliance += value;
                uiState_.brillianceValue = brilliance;
            }
        }

        if(subBass > 50) {uiState_.subBassDetected = true;} else {uiState_.subBassDetected = false;}
        if(bass > 100) {uiState_.bassDetected = true;} else {uiState_.bassDetected = false;}
        if(lowMidRange > 100) {uiState_.lowMidRangeDetected = true;} else {uiState_.lowMidRangeDetected = false;}
        if(midRange > 100) {uiState_.midRangeDetected = true;} else {uiState_.midRangeDetected = false;}
        if(upperMidRange > 100) {uiState_.upperMidRangeDetected = true;} else {uiState_.upperMidRangeDetected = false;}
        if(presence > 100) {uiState_.presenceDetected = true;} else {uiState_.presenceDetected = false;}
        if(brilliance > 100) {uiState_.brillianceDetected = true;} else {uiState_.brillianceDetected = false;}

        if(uiState_.bassDetected) {
            velocityBassReactionTarget_ += 0.5f;
            uiState_.fragmentShaderSettings.bloomBassReaction = 1.0f;
            uiState_.slimeSettings.angleBassReaction = 0;
        } else {
            uiState_.fragmentShaderSettings.bloomBassReaction = 0.0f;
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
    // Use uiState.beatDivide as a decay rate proxy (tune as needed)
    float decayRate = std::max(0.0f, uiState_.beatDivide);
    velocityBassReactionTarget_ *= std::expf(-decayRate * (float)frameTime);

    // clamp
    if(velocityBassReactionSmooth_ < 0.0f) velocityBassReactionSmooth_ = 0.0f;
    if(velocityBassReactionTarget_ < 0.0f) velocityBassReactionTarget_ = 0.0f;

    // write the smoothed value back into uiState so UBO uses it
    uiState_.slimeSettings.velocityBassReaction = velocityBassReactionSmooth_;
}