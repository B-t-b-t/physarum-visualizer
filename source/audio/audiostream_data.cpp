#include "audiostream_data.h"

AudioStreamData::AudioStreamData(SDL_AudioStream* streamId, std::float32_t* buffer, unsigned int bufferSize, SDL_Mutex* mutex, bool hasNewAudioData, bool isShuttingDown) 
    : streamId_(streamId), buffer_(buffer), bufferSize_(bufferSize), mutex_(mutex), hasNewAudioData_(hasNewAudioData), isShuttingDown_(isShuttingDown) {

}

AudioStreamData::AudioStreamData(AudioStreamData&& rhs) {
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

AudioStreamData& AudioStreamData::operator=(AudioStreamData&& rhs) {
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