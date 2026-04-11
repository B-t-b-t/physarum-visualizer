#include "audio_recording.h"

AudioRecording::AudioRecording(std::string deviceName) : deviceName_(deviceName) {
    recordingDevicesArray_ = NULL;

	SDL_Log("Using audio driver: %s", SDL_GetCurrentAudioDriver());
	
	stream_in_ = NULL;
	stream_out_ = NULL;

    bufferSize_ = BUFFER_SIZE;

	//check available devices and populate device vector
	int numDevices = 0;
    recordingDevicesArray_ = SDL_GetAudioRecordingDevices(&numDevices);
    for (int i = 0; recordingDevicesArray_[i] != 0; i++) {
		availableHardwareDevices_.push_back({SDL_GetAudioDeviceName(recordingDevicesArray_[i]), recordingDevicesArray_[i], 0, false});
        const char *name = SDL_GetAudioDeviceName(recordingDevicesArray_[i]);
        SDL_Log("Recording device #%d: '%s'", i, name);
    }

	//select device chosen through program parameters at startup
	bool nothingSelected = true;
	if(deviceName_ != "") {
		for(RecordingDevice& device : availableHardwareDevices_) {
			if(strcmp(device.name, deviceName_.c_str()) == 0) {
				device.selected = true;
				nothingSelected = false;
			} else {
				device.selected = false;
			}
		}
	}

	//autoselect first found recording device
	if(nothingSelected && availableHardwareDevices_.size() > 0) {
		availableHardwareDevices_.at(0).selected = true;
	} else {
		//nothing found
		std::cout << "No recording devices found!" << std::endl;
		availableHardwareDevices_.insert(availableHardwareDevices_.begin(), {"No Recording Device Found", 0, 0, true});
	}

	//copy selected device ID to class member variable
	SDL_AudioDeviceID currentHardwareDevice = 0;
	for(RecordingDevice device : availableHardwareDevices_) {
		if(device.selected) {
			currentHardwareDevice = device.hardwareID;
		}
	}

	//open selected device with SDL
    SDL_AudioDeviceID currentLogicalDevice = SDL_OpenAudioDevice(currentHardwareDevice , NULL);
    if (!currentLogicalDevice) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Couldn't open an audio device for recording: %s!", SDL_GetError());
        SDL_free(recordingDevicesArray_);
    }

	//copy logical device ID into device vector
	for(RecordingDevice device : availableHardwareDevices_) {
		if(device.selected) {
			device.logicalID = currentLogicalDevice;
		} else {
			device.logicalID = 0;
		}
	}

	//print selected device properties for debug
	const char* selectedDeviceName = SDL_GetAudioDeviceName(currentLogicalDevice);
	selectedDeviceName = (selectedDeviceName != nullptr) ? selectedDeviceName : "Unknown";
	std::cout << "Audio Device Name: " << selectedDeviceName << std::endl;
    SDL_GetAudioDeviceFormat(currentLogicalDevice, &inspec_, NULL);

	std::cout << "Audio Format: " << inspec_.format << std::endl;
	std::cout << "Audio Channels: " << inspec_.channels << std::endl;
	std::cout << "Audio Frequency: " << inspec_.freq << std::endl;

	inspec_.channels = 1;	//use just mono (why?)
	audioRate_ = inspec_.freq;

	//create audio data stream with SDL
    stream_in_ = SDL_CreateAudioStream(&inspec_, &inspec_);
    if (!stream_in_) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Couldn't create an audio stream for recording: %s!", SDL_GetError());
    } else if (!SDL_BindAudioStream(currentLogicalDevice, stream_in_)) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Couldn't bind an audio stream for recording: %s!", SDL_GetError());
    }

	SDL_SetAudioStreamGain(stream_in_, 0.2);

	//set Audio Buffer values to 0 (necessary?)
	for(int i = 0; i < BUFFER_SIZE; i++) {
		Buf_[i] = 0;
	}
	audioBuffer_.insert(audioBuffer_.begin(), Buf_, Buf_ + BUFFER_SIZE);


	//set up SDL Timer with seperate Aduio Recording Thread via callback
	mutex_ = SDL_CreateMutex();

	if (!mutex_) {
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Couldn't create mutex\n");
	}
	
	data_ = {stream_in_, Buf_, BUFFER_SIZE, mutex_, false};
	audioTimer_ = 100;

	timerID_ = SDL_AddTimer(audioTimer_, getAudioCallback, &data_);

	std::cout << "Buffer Size: " << sizeof(Buf_) << std::endl;

	std::cout << "Sample Length: " << BUFFER_SIZE / (double)audioRate_ << "s" << std::endl;
	std::cout << "Min Frequency: " << 1 / (BUFFER_SIZE / (double)audioRate_) << "Hz" << std::endl;
	std::cout << "Max Frequency: " << audioRate_ / 2 << "Hz" << std::endl;
	std::cout << "Number of Frequency Bins: " << BUFFER_SIZE / 2 << std::endl;


	spectrum_.reserve(audioProcessor_.getSpectrumSize());
	spectrum_.resize(audioProcessor_.getSpectrumSize(), 0.0);
	previousSpectrum_.reserve(audioProcessor_.getSpectrumSize());
	previousSpectrum_.resize(audioProcessor_.getSpectrumSize(), 0.0);
	spectrumDiff_.reserve(audioProcessor_.getSpectrumSize());
	spectrumDiff_.resize(audioProcessor_.getSpectrumSize(), 0.0);
}

void AudioRecording::selectHardwareDevice(std::string deviceName) {

	SDL_UnbindAudioStream(stream_in_);
	//SDL_DestroyAudioStream(stream_in_);

	SDL_AudioDeviceID currentHardwareDevice = 0;

	//select new device
	for(RecordingDevice& device : availableHardwareDevices_) {
		if(strcmp(device.name, deviceName.c_str()) == 0) {
			device.selected = true;
			currentHardwareDevice = device.hardwareID;
		} else {
			//deselect everything else
			SDL_CloseAudioDevice(device.logicalID);
			device.selected = false;
		}
	}

	//open selected device with SDL
    SDL_AudioDeviceID currentLogicalDevice = SDL_OpenAudioDevice(currentHardwareDevice , NULL);

	if (!currentLogicalDevice) {
    	SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Couldn't open an audio device for recording: %s!", SDL_GetError());
		SDL_free(recordingDevicesArray_);
    }

	//copy logical device ID into device vector
	for(RecordingDevice device : availableHardwareDevices_) {
		if(device.selected) {
			device.logicalID = currentLogicalDevice;
		} else {
			device.logicalID = 0;
		}
	}

	//debug output--------------------------------
	for(RecordingDevice device : availableHardwareDevices_) {
		if(device.selected) {
			std::cout << "Selected Device: " << device.name << std::endl;
		}
	}
	//--------------------------------------------
	
	//print selected device properties for debug
    SDL_GetAudioDeviceFormat(currentLogicalDevice, &inspec_, NULL);
	std::cout << "Audio Format: " << inspec_.format << std::endl;
	std::cout << "Audio Channels: " << inspec_.channels << std::endl;
	std::cout << "Audio Frequency: " << inspec_.freq << std::endl;

	inspec_.channels = 1;	//use just mono (why?)

	audioRate_ = inspec_.freq;

	//bind new device to input stream
	SDL_BindAudioStream(currentLogicalDevice, stream_in_);
}

void AudioRecording::computeSpectrum() {
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

std::vector<std::string> AudioRecording::getAvailableHardwareDeviceNames() {
	std::vector<std::string> availableHardwareDeviceNames;

	for(RecordingDevice device : availableHardwareDevices_) {
		availableHardwareDeviceNames.push_back(device.name);
	}

	return availableHardwareDeviceNames;
}