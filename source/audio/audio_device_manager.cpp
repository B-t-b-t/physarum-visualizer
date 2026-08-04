#include "audio_device_manager.h"

#include <iostream>

AudioDeviceManager::AudioDeviceManager() {
}

AudioDeviceManager::~AudioDeviceManager() {
    closeCurrentDevice();
}

void AudioDeviceManager::checkForAvailableDevices() {
    availableDevices_.clear();  //for repeated calls to this method to check if devices changed

    
    SDL_AudioDeviceID* rawDeviceArray = nullptr;
    int numDevices = 0;
    
    rawDeviceArray = SDL_GetAudioRecordingDevices(&numDevices);
    
    if(!rawDeviceArray || numDevices == 0) {
        SDL_Log("INFO: No recording devices found!");
    } else {
        for (int i = 0; rawDeviceArray[i] != 0; i++) {
            const char* name = SDL_GetAudioDeviceName(rawDeviceArray[i]);
            SDL_Log("Recording device #%d: '%s'", i, name);
            availableDevices_.push_back({name, rawDeviceArray[i]});
        }
    }
    
    if (rawDeviceArray) {
        SDL_free(rawDeviceArray);
    }

    availableDevices_.push_back({"None", 0}); //add default device as last entry
    
    return;
}

bool AudioDeviceManager::openDevice(const std::string& deviceName) {
    closeCurrentDevice();   //just one open device at a time
    if(availableDevices_.empty()) { return false; }

    //if no or false name provided, select first device to be opened
    size_t idx = 0;
    if (!deviceName.empty()) {
        std::optional<size_t> found = findDeviceIndex(deviceName);
        if(found.has_value()) { idx = found.value(); }
    }

    //open device
    currentLogicalID_ = SDL_OpenAudioDevice(availableDevices_[idx].hardwareID, nullptr);
    if(currentLogicalID_ == 0) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Couldn't open audio device: %s \n", SDL_GetError());
        return false;
    }

    //print specs for debug
    SDL_GetAudioDeviceFormat(currentLogicalID_, &currentAudioSpec_, nullptr);
    currentAudioSpec_.channels = 1; // Force mono

    const char* selectedName = SDL_GetAudioDeviceName(currentLogicalID_);
    std::cout << "Opened audio device: " << (selectedName ? selectedName : "Unknown") << std::endl;
    std::cout << "  Format: " << currentAudioSpec_.format
              << " Channels: " << currentAudioSpec_.channels
              << " Frequency: " << currentAudioSpec_.freq << std::endl;

    return true;
}

void AudioDeviceManager::closeCurrentDevice() {
    if (currentLogicalID_ != 0) {
        SDL_CloseAudioDevice(currentLogicalID_);
        currentLogicalID_ = 0;
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