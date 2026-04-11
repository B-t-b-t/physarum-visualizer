#ifndef AUDIO_PROCESSOR_H
#define AUDIO_PROCESSOR_H

#include <complex>
#include <vector>

#include <fftw3.h>

class AudioProcessor {
public:
    AudioProcessor(size_t bufferSize);
    ~AudioProcessor();
    
    std::vector<double> computeSpectrum(const std::vector<double>& audioBuffer);
    size_t getSpectrumSize() const { return spectrumSize_; }

private:
    size_t bufferSize_;
    size_t spectrumSize_;  // bufferSize/2 + 1 (due to Nyquist)
    
    fftw_plan fftPlan_;
    double* inputBuffer_;
    fftw_complex* outputBuffer_;
};

#endif // AUDIO_PROCESSOR_H