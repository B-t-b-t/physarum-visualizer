#ifndef AUDIOSTREAM_DATA_H
#define AUDIOSTREAM_DATA_H

#include <stdfloat>

#include <SDL3/SDL.h>

class AudioStreamData {
public:
	SDL_AudioStream *streamId_ = nullptr;
	std::float32_t* buffer_ = nullptr;
	unsigned int bufferSize_ = 0;
	SDL_Mutex *mutex_ = nullptr;
    bool hasNewAudioData_ = false;
    bool isShuttingDown_ = false;

    AudioStreamData() = default;
    AudioStreamData(SDL_AudioStream* streamId, std::float32_t* buffer, unsigned int bufferSize, SDL_Mutex* mutex, bool hasNewAudioData, bool isShuttingDown);

    AudioStreamData(AudioStreamData&& rhs);
    AudioStreamData& operator=(AudioStreamData&& rhs);
};

#endif  // AUDIOSTREAM_DATA_H