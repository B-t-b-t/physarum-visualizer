#ifndef AUDIOSTREAM_DATA_H
#define AUDIOSTREAM_DATA_H

#include <SDL3/SDL.h>

class AudioStreamData {
public:
	SDL_AudioStream *streamId_ = nullptr;
	short* buffer_ = nullptr;
	unsigned int bufferSize_ = 0;
	SDL_Mutex *mutex_ = nullptr;
    bool hasNewAudioData_ = false;
    bool isShuttingDown_ = false;

    AudioStreamData() = default;
    AudioStreamData(SDL_AudioStream* streamId, short* buffer, unsigned int bufferSize, SDL_Mutex* mutex, bool hasNewAudioData, bool isShuttingDown);

    AudioStreamData(AudioStreamData&& rhs);
    AudioStreamData& operator=(AudioStreamData&& rhs);
};

#endif  // AUDIOSTREAM_DATA_H