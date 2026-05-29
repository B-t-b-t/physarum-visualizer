#include "audio_system.h"

AudioSystem::AudioSystem(std::string deviceName) {	

	//check available devices and populate device vector
	deviceManager_.checkForAvailableDevices();
	bool success = deviceManager_.openDevice(deviceName);

	if(success) {
		inSpec_ = deviceManager_.getCurrentAudioSpec();
		inSpec_.channels = 1;	// Force mono

		//create audio data stream with SDL
		SDL_AudioStream *streamIn = SDL_CreateAudioStream(&inSpec_, &inSpec_);
		if (!streamIn) {
			SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Couldn't create an audio stream for recording: %s!", SDL_GetError());
		} else if (!SDL_BindAudioStream(deviceManager_.getCurrentLogicalDeviceID(), streamIn)) {
			SDL_DestroyAudioStream(streamIn);
			streamIn = nullptr;
			SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Couldn't bind an audio stream for recording: %s!", SDL_GetError());
		}

		SDL_SetAudioStreamGain(streamIn, 0.2);

		//set Audio Buffer values to 0 (necessary?)
		for(int i = 0; i < BUFFER_SIZE; i++) {
			Buf_[i] = 0;
		}
		audioBuffer_.insert(audioBuffer_.begin(), Buf_, Buf_ + BUFFER_SIZE);


		//set up SDL Timer with seperate Audio Recording Thread via callback
		SDL_Mutex *mutex = SDL_CreateMutex();

		if (!mutex) {
			SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Couldn't create mutex\n");
		}
		
		data_ = {streamIn, Buf_, BUFFER_SIZE, mutex, false, false};
		audioTimer_ = 100;

		timerID_ = SDL_AddTimer(audioTimer_, getAudioCallback, &data_);
		if(timerID_ == 0) {
			std::cerr << SDL_GetError() << std::endl;
		}

		std::cout << "Buffer Size: " << sizeof(Buf_) << std::endl;

		std::cout << "Sample Length: " << BUFFER_SIZE / (double)getAudioRate() << "s" << std::endl;
		std::cout << "Min Frequency: " << 1 / (BUFFER_SIZE / (double)getAudioRate()) << "Hz" << std::endl;
		std::cout << "Max Frequency: " << getAudioRate() / 2 << "Hz" << std::endl;
		std::cout << "Number of Frequency Bins: " << BUFFER_SIZE / 2 << std::endl;


		spectrum_.assign(audioProcessor_.getSpectrumSize(), 0.0);
		previousSpectrum_.assign(audioProcessor_.getSpectrumSize(), 0.0);
		spectrumDiff_.assign(audioProcessor_.getSpectrumSize(), 0.0);
	}
}

AudioSystem::~AudioSystem() {

	if(data_.mutex != nullptr) {
		SDL_LockMutex(data_.mutex);
		data_.isShuttingDown = true;	//signal audio callback to stop timer and exit
		data_.streamId = nullptr;
		SDL_UnlockMutex(data_.mutex);
	}

	if(timerID_ != 0) {
		bool isTimerRemoved = SDL_RemoveTimer(timerID_);
		if (!isTimerRemoved) {
			std::cerr << "Couldn't remove timer: " << SDL_GetError() << std::endl;
		}
	}

	if(data_.mutex != nullptr) {
		SDL_LockMutex(data_.mutex);	//wait for final audiocallback to stop if running
		SDL_UnlockMutex(data_.mutex);
	}
	
	if(data_.streamId != nullptr) {
		SDL_DestroyAudioStream(data_.streamId);
		data_.streamId = nullptr;
	}
	
	if(data_.mutex != nullptr) {
		SDL_DestroyMutex(data_.mutex);
		data_.mutex = nullptr;
	}
}

void AudioSystem::selectHardwareDevice(std::string deviceName) {

	SDL_UnbindAudioStream(data_.streamId);
	//SDL_DestroyAudioStream(data_.streamId);

	deviceManager_.openDevice(deviceName);
	inSpec_ = deviceManager_.getCurrentAudioSpec();

	inSpec_.channels = 1;	//use just mono (why?)

	//bind new device to input stream
	SDL_BindAudioStream(deviceManager_.getCurrentLogicalDeviceID(), data_.streamId);
}

void AudioSystem::computeSpectrum() {
	if(data_.mutex == nullptr) { return; }

    if (SDL_TryLockMutex(data_.mutex)) {
		if(data_.hasNewAudioData) {
        	audioBuffer_.clear();
        	audioBuffer_.insert(audioBuffer_.begin(), Buf_, Buf_ + BUFFER_SIZE);
			data_.hasNewAudioData = false;
			hasNewAudioData_ = true;
		}
        SDL_UnlockMutex(data_.mutex);
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

	if(data == nullptr || data->mutex == nullptr) {
		std::cerr << "Audio Callback: No data or mutex provided!" << std::endl;
		return 0;	//stop callback
	}
	
	SDL_LockMutex(data->mutex);

	if(data->isShuttingDown || data->streamId == nullptr || data->buffer == nullptr) { 
		SDL_UnlockMutex(data->mutex);
		return 0; 	//stop timer if shutting down
	}

	timerID = timerID;		// to avoid pedantic warning
		
	for(unsigned int i = 0; i < data->bufferSize; i++) {
		(data->buffer)[i] = 0;
	}
	
	int read = SDL_GetAudioStreamData(data->streamId, data->buffer, data->bufferSize * sizeof(*data->buffer));
	if(read == -1) {
		std::cerr << SDL_GetError() << std::endl;
	}

	bool isStreamCleared = SDL_ClearAudioStream(data->streamId);
	if(!isStreamCleared) {
		std::cerr << SDL_GetError() << std::endl;
	}

	data->hasNewAudioData = true;	//for synchronizing spectrum computation in main loop
	SDL_UnlockMutex(data->mutex);
	
	return intervall;
}