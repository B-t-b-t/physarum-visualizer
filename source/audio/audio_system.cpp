#include "audio_system.h"

AudioSystem::AudioSystem(std::string deviceName) {	
	stream_in_ = NULL;

    bufferSize_ = BUFFER_SIZE;

	//check available devices and populate device vector
	deviceManager_.checkForAvailableDevices();
	bool success = deviceManager_.openDevice(deviceName);

	if(success) {
		inSpec_ = deviceManager_.getCurrentAudioSpec();
		inSpec_.channels = 1;	// Force mono

		//create audio data stream with SDL
		stream_in_ = SDL_CreateAudioStream(&inSpec_, &inSpec_);
		if (!stream_in_) {
			SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Couldn't create an audio stream for recording: %s!", SDL_GetError());
		} else if (!SDL_BindAudioStream(deviceManager_.getCurrentLogicalDeviceID(), stream_in_)) {
			SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Couldn't bind an audio stream for recording: %s!", SDL_GetError());
		}

		SDL_SetAudioStreamGain(stream_in_, 0.2);

		//set Audio Buffer values to 0 (necessary?)
		for(int i = 0; i < BUFFER_SIZE; i++) {
			Buf_[i] = 0;
		}
		audioBuffer_.insert(audioBuffer_.begin(), Buf_, Buf_ + BUFFER_SIZE);


		//set up SDL Timer with seperate Audio Recording Thread via callback
		mutex_ = SDL_CreateMutex();

		if (!mutex_) {
			SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Couldn't create mutex\n");
		}
		
		data_ = {stream_in_, Buf_, BUFFER_SIZE, mutex_, false};
		audioTimer_ = 100;

		timerID_ = SDL_AddTimer(audioTimer_, getAudioCallback, &data_);

		std::cout << "Buffer Size: " << sizeof(Buf_) << std::endl;

		std::cout << "Sample Length: " << BUFFER_SIZE / (double)getAudioRate() << "s" << std::endl;
		std::cout << "Min Frequency: " << 1 / (BUFFER_SIZE / (double)getAudioRate()) << "Hz" << std::endl;
		std::cout << "Max Frequency: " << getAudioRate() / 2 << "Hz" << std::endl;
		std::cout << "Number of Frequency Bins: " << BUFFER_SIZE / 2 << std::endl;


		spectrum_.reserve(audioProcessor_.getSpectrumSize());
		spectrum_.resize(audioProcessor_.getSpectrumSize(), 0.0);
		previousSpectrum_.reserve(audioProcessor_.getSpectrumSize());
		previousSpectrum_.resize(audioProcessor_.getSpectrumSize(), 0.0);
		spectrumDiff_.reserve(audioProcessor_.getSpectrumSize());
		spectrumDiff_.resize(audioProcessor_.getSpectrumSize(), 0.0);
	}
}

AudioSystem::~AudioSystem() {
	bool isTimerRemoved = SDL_RemoveTimer(timerID_);
	if (!isTimerRemoved) {
		std::cerr << SDL_GetError() << std::endl;
	}

	SDL_UnlockMutex(mutex_);
	SDL_DestroyMutex(mutex_);

	SDL_UnbindAudioStream(stream_in_);
	SDL_DestroyAudioStream(stream_in_);
}

void AudioSystem::selectHardwareDevice(std::string deviceName) {

	SDL_UnbindAudioStream(stream_in_);
	//SDL_DestroyAudioStream(stream_in_);

	deviceManager_.openDevice(deviceName);
	inSpec_ = deviceManager_.getCurrentAudioSpec();

	inSpec_.channels = 1;	//use just mono (why?)

	//bind new device to input stream
	SDL_BindAudioStream(deviceManager_.getCurrentLogicalDeviceID(), stream_in_);
}

void AudioSystem::computeSpectrum() {
    if (SDL_TryLockMutex(mutex_)) {
		if(data_.hasNewAudioData) {
        	audioBuffer_.clear();
        	audioBuffer_.insert(audioBuffer_.begin(), Buf_, Buf_ + BUFFER_SIZE);
			data_.hasNewAudioData = false;
			hasNewAudioData_ = true;
		}
        SDL_UnlockMutex(mutex_);
    } else {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Couldn't lock mutex in AudioBuffer Vector Insert\n");
    }
	
	if(hasNewAudioData_) {
    	spectrum_ = audioProcessor_.computeSpectrum(audioBuffer_);

		//compute spectrum difference for beat detection
		for(unsigned int i = 0; i < spectrum_.size(); i++) {
			spectrumDiff_[i] = spectrum_[i] - previousSpectrum_[i];
			previousSpectrum_[i] = spectrum_[i];
		}

		hasNewAudioData_ = false;
		hasNewSpectrumData_ = true;
	}
}

Uint32 getAudioCallback (void *parameter, SDL_TimerID timerID, Uint32 intervall) {
	AudioStreamData* data = (AudioStreamData*)parameter;

	SDL_Mutex *mutex = data->mutex;

	SDL_LockMutex(mutex);

	timerID = timerID;		// to avoid pedantic warning
		
	for(unsigned int i = 0; i < data->bufferSize; i++) {
		(data->buffer)[i] = 0;
	}
	
	SDL_GetAudioStreamData(data->streamId, data->buffer, data->bufferSize * sizeof(*data->buffer));
	SDL_ClearAudioStream(data->streamId);

	data->hasNewAudioData = true;	//for synchronizing spectrum computation in main loop

	SDL_UnlockMutex(mutex);
	 
	//SDL_Log("Timer: %llu", (unsigned long long)SDL_GetTicks());
	return intervall;
}