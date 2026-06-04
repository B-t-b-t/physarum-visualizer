#ifndef AUDIO_SYSTEM_H
#define AUDIO_SYSTEM_H

#include <iostream>
#include <vector>

#include <SDL3/SDL.h>

#include "audio_processor.h"
#include "audio_device_manager.h"

#define BUFFER_SIZE 4096//2048 //65536

struct AudioStreamData {
	SDL_AudioStream *streamId_ = nullptr;
	short* buffer_ = nullptr;
	unsigned int bufferSize_ = 0;
	SDL_Mutex *mutex_ = nullptr;
    bool hasNewAudioData_ = false;
    bool isShuttingDown_ = false;

    AudioStreamData() = default;
    AudioStreamData(SDL_AudioStream* streamId, short* buffer, unsigned int bufferSize, SDL_Mutex* mutex, bool hasNewAudioData, bool isShuttingDown) 
        : streamId_(streamId), buffer_(buffer), bufferSize_(bufferSize), mutex_(mutex), hasNewAudioData_(hasNewAudioData), isShuttingDown_(isShuttingDown) {

    }

    AudioStreamData(AudioStreamData&& rhs) {
        this->streamId_ = rhs.streamId_;
        this->buffer_ = rhs.buffer_;
        this->bufferSize_ = rhs.bufferSize_;
        this->mutex_ = rhs.mutex_;
        this->hasNewAudioData_ = rhs.hasNewAudioData_;
        this->isShuttingDown_ = rhs.isShuttingDown_;

        rhs.streamId_ = nullptr;
        rhs.buffer_ = nullptr;
        rhs.bufferSize_ = 0;
        rhs.mutex_ = nullptr;
        rhs.hasNewAudioData_ = false;
        rhs.isShuttingDown_ = false;
    }

    AudioStreamData& operator=(AudioStreamData&& rhs) {
        if (this != &rhs) {
            this->streamId_ = rhs.streamId_;
            this->buffer_ = rhs.buffer_;
            this->bufferSize_ = rhs.bufferSize_;
            this->mutex_ = rhs.mutex_;
            this->hasNewAudioData_ = rhs.hasNewAudioData_;
            this->isShuttingDown_ = rhs.isShuttingDown_;

            rhs.streamId_ = nullptr;
            rhs.buffer_ = nullptr;
            rhs.bufferSize_ = 0;
            rhs.mutex_ = nullptr;
            rhs.hasNewAudioData_ = false;
            rhs.isShuttingDown_ = false;
        }
        return *this;
    }
};

class AudioSystem {
public:
    AudioSystem(std::string deviceName = "");

    ~AudioSystem();

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

private:
    AudioDeviceManager deviceManager_;

    SDL_AudioSpec inSpec_;
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

    bool hasNewAudioData_ = false;
    bool hasNewSpectrumData_ = false;
};

Uint32 getAudioCallback (void *parameter, SDL_TimerID timerID, Uint32 intervall);

#endif // AUDIO_SYSTEM_H