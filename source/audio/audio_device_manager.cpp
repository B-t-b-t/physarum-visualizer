#include "audio_device_manager.h"

#include <iostream>

AudioDeviceManager::AudioDeviceManager() {
}

AudioDeviceManager::~AudioDeviceManager() {
    closeCurrentDevice();
}

bool AudioDeviceManager::checkForAvailableDevices() {

    std::vector<AudioDeviceInfo> temp = availableDevices_;

    availableDevices_.clear();  //for repeated calls to this method to check if devices changed
    usedDeviceIndex_.reset();
    
    SDL_AudioDeviceID* rawDeviceArray = nullptr;
    int numDevices = 0;
    
    rawDeviceArray = SDL_GetAudioRecordingDevices(&numDevices);
    
    if(!rawDeviceArray || numDevices == 0) {
        SDL_Log("INFO: No recording devices found!");
    } else {
        for (int i = 0; i < numDevices; i++) {
            const char* name = SDL_GetAudioDeviceName(rawDeviceArray[i]);
            if(name == nullptr) {
                SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Couldn't get name for audio device #%d: %s", i, SDL_GetError());
            }

            SDL_AudioSpec spec{};
            bool formatSuccess = SDL_GetAudioDeviceFormat(rawDeviceArray[i], &spec, nullptr);
            if(!formatSuccess) {
                SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Couldn't get format for audio device #%d: %s", i, SDL_GetError());
            }

            if(name != nullptr && formatSuccess) {
                availableDevices_.push_back({name, rawDeviceArray[i], 0, spec});
                SDL_Log("Recording device #%d (Hardware-ID: %d): '%s'", i, rawDeviceArray[i], name);
            }
        }
    }
    
    if (rawDeviceArray) {
        SDL_free(rawDeviceArray);   //allocated by SDL, must be freed manually
    }

    availableDevices_.push_back({"No Device", 0, 0, SDL_AudioSpec{}}); //add default device as last entry

    bool areDevicesOpen = false;

    //restore logical IDs
    for(size_t i = 0; i < availableDevices_.size(); ++i) {
        for(size_t j = 0; j < temp.size(); ++j) {
            if(availableDevices_[i].hardwareID == temp[j].hardwareID) {
                availableDevices_[i].logicalID = temp[j].logicalID;
                //check for open devices
                if(availableDevices_[i].logicalID != 0) {
                    areDevicesOpen = true;
                    usedDeviceIndex_ = i;
                }
                break;
            }
        }
    }

    return areDevicesOpen;  //used for closing audiostreams if bound device was removed
}

bool AudioDeviceManager::openDevice(const std::string& deviceName) {
    closeCurrentDevice();   //just one open device at a time
    if(availableDevices_.empty() || deviceName == "No Device") { return false; }

    //if no or false name provided, select first device to be opened
    size_t idx = 0;
    if (!deviceName.empty()) {
        std::optional<size_t> found = findDeviceIndex(deviceName);
        if(found.has_value()) { idx = found.value(); }
    }

    //open device
    SDL_AudioDeviceID logicalID = 0;
    logicalID = SDL_OpenAudioDevice(availableDevices_[idx].hardwareID, nullptr);
    if(logicalID != 0) {
        availableDevices_[idx].logicalID = logicalID;
        usedDeviceIndex_ = idx;

        std::cout << "Opened audio device: " << availableDevices_[*usedDeviceIndex_].name << std::endl;
        std::cout << "  Format: " << SDL_GetAudioFormatName(availableDevices_[*usedDeviceIndex_].currentAudioSpec_.format)
                  << " Channels: " << availableDevices_[*usedDeviceIndex_].currentAudioSpec_.channels
                  << " Frequency: " << availableDevices_[*usedDeviceIndex_].currentAudioSpec_.freq << std::endl;

    } else {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Couldn't open audio device: %s \n", SDL_GetError());
    }

    return logicalID != 0;
}

void AudioDeviceManager::closeCurrentDevice() {
    if(usedDeviceIndex_) {
        SDL_CloseAudioDevice(availableDevices_[*usedDeviceIndex_].logicalID);
        availableDevices_[*usedDeviceIndex_].logicalID = 0;
        usedDeviceIndex_.reset();
    }
}

std::vector<std::string> AudioDeviceManager::getAvailableDeviceNames() {
    checkForAvailableDevices();  //refresh device list
    std::vector<std::string> names;
    names.reserve(availableDevices_.size());
    for (const auto& device : availableDevices_) {
        names.push_back(device.name);
    }
    return names;
}

std::optional<size_t> AudioDeviceManager::findDeviceIndex(const std::string& name) const {
    for (size_t i = 0; i < availableDevices_.size(); i++) {
        if (availableDevices_[i].name == name) return i;
    }
    return std::nullopt;
}

bool AudioDeviceManager::processAudioDeviceEvents() {
    SDL_Event audioDeviceEvent;
    bool areDevicesOpen = usedDeviceIndex_.has_value() && availableDevices_[*usedDeviceIndex_].logicalID != 0; //check if any device is open

    SDL_PumpEvents(); //necessary to update the event queue with latest events

    //filter just audio device events
	while (SDL_PeepEvents(&audioDeviceEvent, 1, SDL_GETEVENT, SDL_EVENT_AUDIO_DEVICE_ADDED, SDL_EVENT_AUDIO_DEVICE_FORMAT_CHANGED) > 0) {

		switch (audioDeviceEvent.type) {
            case SDL_EVENT_AUDIO_DEVICE_ADDED:
                areDevicesOpen = checkForAvailableDevices(); //refresh device list
                SDL_Log("Audio device added: %u", audioDeviceEvent.adevice.which);
                break;
            case SDL_EVENT_AUDIO_DEVICE_REMOVED:
                areDevicesOpen = checkForAvailableDevices(); //refresh device list
                SDL_Log("Audio device removed: %u", audioDeviceEvent.adevice.which);
                break;
            case SDL_EVENT_AUDIO_DEVICE_FORMAT_CHANGED:
                areDevicesOpen = checkForAvailableDevices(); //refresh device list
                SDL_Log("Audio device format changed: %u", audioDeviceEvent.adevice.which);
                break;
            default:
                break;
		}
	}
    return areDevicesOpen;
}