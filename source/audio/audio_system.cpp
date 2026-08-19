#include "audio_system.h"

AudioSystem::AudioSystem(ApplicationState* appState, std::string deviceName) 
 : appState_{appState} 
{

	//check available devices and populate device vector
	deviceManager_.checkForAvailableDevices();
	appState_->availableAudioHardware = deviceManager_.getAvailableDevices();
	bool isDeviceOpen = deviceManager_.openDevice(deviceName);

	if(isDeviceOpen) {
		inSpec_ = deviceManager_.getCurrentAudioSpec();
		outSpec_.format = SDL_AUDIO_F32LE;	//use float for FFT
		outSpec_.channels = 1;	//use just mono for FFT
		outSpec_.freq = inSpec_.freq;

		//create audio data stream with SDL
		SDL_AudioStream *streamIn = SDL_CreateAudioStream(&inSpec_, &inSpec_);
		if (streamIn != nullptr) {

			if (SDL_BindAudioStream(deviceManager_.getCurrentLogicalDeviceID(), streamIn)) {
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
				
				data_ = AudioStreamData(streamIn, Buf_, BUFFER_SIZE, mutex, false, false);
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

				appState_->audioBuffer = &audioBuffer_;
				appState_->spectrum = &spectrum_;
				appState_->spectrumDiff = &spectrumDiff_;
				appState_->bufferSize = BUFFER_SIZE;
				appState_->hasNewSpectrumData = &hasNewSpectrumData_;
			} else {
				SDL_DestroyAudioStream(streamIn);
				streamIn = nullptr;
				SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Couldn't bind an audio stream for recording: %s!", SDL_GetError());
			}

		} else {
			SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Couldn't create an audio stream for recording: %s!", SDL_GetError());
		}
	}
}

AudioSystem::~AudioSystem() {

	if(data_.mutex_ != nullptr) {
		SDL_LockMutex(data_.mutex_);
		data_.isShuttingDown_ = true;	//signal audio callback to stop timer and exit
		SDL_UnlockMutex(data_.mutex_);
	}

	if(timerID_ != 0) {
		bool isTimerRemoved = SDL_RemoveTimer(timerID_);
		if (!isTimerRemoved) {
			std::cerr << "Couldn't remove timer: " << SDL_GetError() << std::endl;
		}
	}

	if(data_.mutex_ != nullptr) {
		SDL_LockMutex(data_.mutex_);	//wait for final audiocallback to stop if running
		SDL_UnlockMutex(data_.mutex_);
	}
	
	if(data_.streamId_ != nullptr) {
		SDL_DestroyAudioStream(data_.streamId_);
		data_.streamId_ = nullptr;
	}
	
	if(data_.mutex_ != nullptr) {
		SDL_DestroyMutex(data_.mutex_);
		data_.mutex_ = nullptr;
	}
}

void AudioSystem::onNotify(const Event event) {
	if(event == Event::AUDIO_HARDWARE_CHANGE) {
		selectHardwareDevice(appState_->availableAudioHardware->at(appState_->usedAudioHardwareIndex).name);
	}
}

void AudioSystem::selectHardwareDevice(std::string deviceName) {

	SDL_UnbindAudioStream(data_.streamId_);
	//SDL_DestroyAudioStream(data_.streamId_);

	bool isOpen = deviceManager_.openDevice(deviceName);
	inSpec_ = deviceManager_.getCurrentAudioSpec();

	//bind new device to input stream
	if(isOpen) {
		SDL_BindAudioStream(deviceManager_.getCurrentLogicalDeviceID(), data_.streamId_);
		//appState_->slimeSettings.reactToAudio = true;	//turn on audio reaction if a device is available
	} else {
		hasNewAudioData_ = false;
		hasNewSpectrumData_ = false;
		appState_->slimeSettings.reactToAudio = false;	//turn off audio reaction if no device is available
	}
}

void AudioSystem::update() {
	bool areDevicesOpen = deviceManager_.processAudioDeviceEvents();

	if(!areDevicesOpen) {
		selectHardwareDevice("No Device");	//close device if no devices are available
	}
}

void AudioSystem::computeSpectrum() {
	if(data_.mutex_ == nullptr) { return; }

    if (SDL_TryLockMutex(data_.mutex_)) {
		if(data_.hasNewAudioData_) {
        	audioBuffer_.clear();
        	audioBuffer_.insert(audioBuffer_.begin(), Buf_, Buf_ + BUFFER_SIZE);
			data_.hasNewAudioData_ = false;
			hasNewAudioData_ = true;
		}
        SDL_UnlockMutex(data_.mutex_);
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