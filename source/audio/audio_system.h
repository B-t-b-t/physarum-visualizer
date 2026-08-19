#ifndef AUDIO_SYSTEM_H
#define AUDIO_SYSTEM_H

#include <iostream>
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
    AudioSystem(ApplicationState* appState, std::string deviceName = "");

    ~AudioSystem();

    void update();
    void computeSpectrum();

    int getAudioRate() const { return inSpec_.freq; }
    Uint32 getAudioTimer() { return audioTimer_; }
    void setAudioTimer(Uint32 audioTimer) { audioTimer_ = audioTimer; }
    std::vector<double>& getAudioBuffer() { return audioBuffer_; }
    std::vector<double>& getSpectrum() { return spectrum_; }
    std::vector<double>& getSpectrumDiff() { return spectrumDiff_; }
    int getBufferSize() const { return bufferSize_; }

    std::vector<std::string> getAvailableHardwareDeviceNames() { return deviceManager_.getAvailableDeviceNames(); }
    int getNumAvailableHardwareDevices() { return deviceManager_.getNumAvailableDevices(); }
    void selectHardwareDevice(std::string deviceName);

    bool hasNewAudioData() const { return hasNewAudioData_; }
    void setHasNewAudioData(bool hasNew) { hasNewAudioData_ = hasNew; }

    bool hasNewSpectrumData() const { return hasNewSpectrumData_; }
    void setHasNewSpectrumData(bool hasNew) { hasNewSpectrumData_ = hasNew; }

    void onNotify(const Event event) override;

private:
    AudioDeviceManager deviceManager_;

    SDL_AudioSpec inSpec_{};
    SDL_AudioSpec outSpec_{};
	AudioStreamData data_;

    int bufferSize_ = BUFFER_SIZE;
    std::vector<double> audioBuffer_;
	int16_t Buf_[BUFFER_SIZE];

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