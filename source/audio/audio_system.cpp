#include "audio_system.h"

AudioSystem::AudioSystem(ApplicationState* appState) 
 : appState_{appState} 
{
	//check available devices and populate device vector
	appState_->availableAudioHardware = deviceManager_.getAvailableDevices();

	outSpec_.format = SDL_AUDIO_F32LE;	//use float for FFT
	outSpec_.channels = 1;	//use just mono for FFT
	outSpec_.freq = 48000;	//use 48kHz for FFT

	audioStreamData_ = AudioStreamData(nullptr, Buf_, BUFFER_SIZE, nullptr, false, false);

	spectrum_.assign(audioProcessor_.getSpectrumSize(), 0.0);
	previousSpectrum_.assign(audioProcessor_.getSpectrumSize(), 0.0);
	spectrumDiff_.assign(audioProcessor_.getSpectrumSize(), 0.0);
	
	appState_->audioBuffer = &audioBuffer_;
	appState_->spectrum = &spectrum_;
	appState_->spectrumDiff = &spectrumDiff_;
	appState_->bufferSize = BUFFER_SIZE;
	appState_->hasNewSpectrumData = &hasNewSpectrumData_;
}

AudioSystem::~AudioSystem() {

	if(audioStreamData_.mutex_ != nullptr) {
		SDL_LockMutex(audioStreamData_.mutex_);
		audioStreamData_.isShuttingDown_ = true;	//signal audio callback to stop timer and exit
		SDL_UnlockMutex(audioStreamData_.mutex_);
	}

	if(timerID_ != 0) {
		bool isTimerRemoved = SDL_RemoveTimer(timerID_);
		if (!isTimerRemoved) {
			std::cerr << "Couldn't remove timer: " << SDL_GetError() << std::endl;
		}
	}

	if(audioStreamData_.mutex_ != nullptr) {
		SDL_LockMutex(audioStreamData_.mutex_);	//wait for final audiocallback to stop if running
		SDL_UnlockMutex(audioStreamData_.mutex_);
	}
	
	if(audioStreamData_.streamId_ != nullptr) {
		SDL_DestroyAudioStream(audioStreamData_.streamId_);
		audioStreamData_.streamId_ = nullptr;
	}
	
	if(audioStreamData_.mutex_ != nullptr) {
		SDL_DestroyMutex(audioStreamData_.mutex_);
		audioStreamData_.mutex_ = nullptr;
	}
}

void AudioSystem::onNotify(const Event event) {
	if(event == Event::AUDIO_HARDWARE_CHANGE) {
		selectRecordingDevice(appState_->usedAudioHardwareIndex);
	}
}

void AudioSystem::createAudioStream() {
	inSpec_ = deviceManager_.getOpenDevice().value().audioSpec;

	//create audio data stream with SDL
	SDL_AudioStream *streamIn = SDL_CreateAudioStream(&inSpec_, &outSpec_);

	if (streamIn != nullptr) {
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
		
		audioStreamData_.streamId_ = streamIn;
		audioStreamData_.mutex_ = mutex;

		audioTimer_ = 100;

		timerID_ = SDL_AddTimer(audioTimer_, getAudioCallback, &audioStreamData_);
		if(timerID_ == 0) {
			std::cerr << SDL_GetError() << std::endl;
		}

		std::cout << "Buffer Size: " << sizeof(Buf_) << std::endl;

		std::cout << "Sample Length: " << BUFFER_SIZE / (double)inSpec_.freq << "s" << std::endl;
		std::cout << "Min Frequency: " << 1 / (BUFFER_SIZE / (double)inSpec_.freq) << "Hz" << std::endl;
		std::cout << "Max Frequency: " << inSpec_.freq / 2 << "Hz" << std::endl;
		std::cout << "Number of Frequency Bins: " << BUFFER_SIZE / 2 << std::endl;
	} else {
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Couldn't create an audio stream for recording: %s!", SDL_GetError());
	}
}

void AudioSystem::selectRecordingDevice(SDL_AudioDeviceID selectedDeviceID) {
	
    SDL_UnbindAudioStream(audioStreamData_.streamId_);

    if (selectedDeviceID == 0 || !deviceManager_.openDevice(selectedDeviceID)) {
        hasNewAudioData_ = false;
        hasNewSpectrumData_ = false;
        appState_->slimeSettings.reactToAudio = false;
        return;
    }

    inSpec_ = deviceManager_.getOpenDevice().value().audioSpec;

    if (audioStreamData_.streamId_ == nullptr) {
        createAudioStream();
    }

    if (audioStreamData_.streamId_ == nullptr ||
        !SDL_BindAudioStream(deviceManager_.getOpenDevice().value().logicalID, audioStreamData_.streamId_)) {
        hasNewAudioData_ = false;
        hasNewSpectrumData_ = false;
        appState_->slimeSettings.reactToAudio = false;
        SDL_LogError(
            SDL_LOG_CATEGORY_APPLICATION,
            "Couldn't bind audio stream for recording: %s",
            SDL_GetError()
        );
        return;
    }
}

void AudioSystem::update() {
	bool openDeviceUnchanged = deviceManager_.processAudioDeviceChanges();

	if(!openDeviceUnchanged) {
		selectRecordingDevice(0);
	}
}

void AudioSystem::computeSpectrum() {
	if(audioStreamData_.mutex_ == nullptr) { return; }

    if (SDL_TryLockMutex(audioStreamData_.mutex_)) {
		if(audioStreamData_.hasNewAudioData_) {
        	audioBuffer_.clear();
        	audioBuffer_.insert(audioBuffer_.begin(), Buf_, Buf_ + BUFFER_SIZE);
			audioStreamData_.hasNewAudioData_ = false;
			hasNewAudioData_ = true;
		}
        SDL_UnlockMutex(audioStreamData_.mutex_);
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

	if(data == nullptr || data->mutex_ == nullptr) {
		std::cerr << "Audio Callback: No data or mutex provided!" << std::endl;
		return 0;	//stop callback
	}
	
	SDL_LockMutex(data->mutex_);

	if(data->isShuttingDown_ || data->streamId_ == nullptr || data->buffer_ == nullptr) { 
		SDL_UnlockMutex(data->mutex_);
		return 0; 	//stop timer if shutting down
	}

	timerID = timerID;		// to avoid pedantic warning
		
	for(unsigned int i = 0; i < data->bufferSize_; i++) {
		(data->buffer_)[i] = 0;
	}
	
	int read = SDL_GetAudioStreamData(data->streamId_, data->buffer_, data->bufferSize_ * sizeof(*data->buffer_));
	if(read == -1) {
		std::cerr << SDL_GetError() << std::endl;
	}

	bool isStreamCleared = SDL_ClearAudioStream(data->streamId_);
	if(!isStreamCleared) {
		std::cerr << SDL_GetError() << std::endl;
	}

	data->hasNewAudioData_ = true;	//for synchronizing spectrum computation in main loop
	SDL_UnlockMutex(data->mutex_);
	
	return intervall;
}