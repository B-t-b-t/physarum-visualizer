#include "AudioProcessor.h"
#include <cstring>
#include <cmath>

AudioProcessor::AudioProcessor(size_t bufferSize) 
    : bufferSize_(bufferSize)
    , spectrumSize_(bufferSize / 2 + 1) {
    
    // Allocate FFTW buffers
    inputBuffer_ = static_cast<double*>(fftw_malloc(bufferSize_ * sizeof(double)));
    outputBuffer_ = static_cast<fftw_complex*>(fftw_malloc(spectrumSize_ * sizeof(fftw_complex)));
    
    // Create FFT plan
    fftPlan_ = fftw_plan_dft_r2c_1d(
        bufferSize_,
        inputBuffer_,
        outputBuffer_,
        FFTW_ESTIMATE
    );
}

AudioProcessor::~AudioProcessor() {
    fftw_destroy_plan(fftPlan_);
    fftw_free(outputBuffer_);
    fftw_free(inputBuffer_);
}

std::vector<double> AudioProcessor::computeSpectrum(const std::vector<double>& audioBuffer) {
    // Copy input data
    std::memcpy(inputBuffer_, audioBuffer.data(), bufferSize_ * sizeof(double));
    
    // Execute FFT
    fftw_execute(fftPlan_);
    
    // Convert complex output to magnitude spectrum
    std::vector<double> spectrum(spectrumSize_);

    double maxVal = 0.0;

    for(size_t i = 0; i < spectrumSize_; i++) {
        float real = outputBuffer_[i][0];
        float imag = outputBuffer_[i][1];
        spectrum[i] = std::sqrt(real * real + imag * imag);
        
        maxVal = std::max(maxVal, spectrum[i]);
    }

    // Normalize
    for(size_t i = 0; i < spectrumSize_; i++) {
        spectrum[i] = (spectrum[i] / maxVal) * 100.0;
    }
    
    return spectrum;
}