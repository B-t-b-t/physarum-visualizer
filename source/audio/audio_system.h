#ifndef AUDIO_SYSTEM_H
#define AUDIO_SYSTEM_H

#include <iostream>
#include <stdfloat>
#include <vector>

#include <SDL3/SDL.h>

#include "audio_processor.h"
#include "audio_device_manager.h"
#include "audiostream_data.h"
#include "../application_state.h"
#include "../utility/observer.h"

#define BUFFER_SIZE 4096//2048 //65536

class AudioSystem : public Observer{
public:
    AudioSystem(ApplicationState* appState);
    ~AudioSystem();

    void update();
    void computeSpectrum();

    std::vector<double>& getSpectrumDiff() { return spectrumDiff_; }

    void selectRecordingDevice(SDL_AudioDeviceID hardwareID);

    void onNotify(const Event event) override;

    void createAudioStream();

private:
    AudioDeviceManager deviceManager_;

    SDL_AudioSpec inSpec_{};
    SDL_AudioSpec outSpec_{};
	AudioStreamData audioStreamData_;

    int bufferSize_ = BUFFER_SIZE;
    std::vector<double> audioBuffer_;
	std::float32_t Buf_[BUFFER_SIZE];

    AudioProcessor audioProcessor_ = AudioProcessor(BUFFER_SIZE);
	std::vector<double> spectrum_;
    std::vector<double> previousSpectrum_;
    std::vector<double> spectrumDiff_;

    Uint32 audioTimer_;
    SDL_TimerID timerID_ = 0;

    ApplicationState* appState_;

    bool hasNewAudioData_ = false;
    bool hasNewSpectrumData_ = false;
};

Uint32 getAudioCallback (void *parameter, SDL_TimerID timerID, Uint32 intervall);

#endif // AUDIO_SYSTEM_H