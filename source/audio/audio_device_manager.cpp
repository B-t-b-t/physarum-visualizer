#include "audio_device_manager.h"

#include <iostream>

AudioDeviceManager::AudioDeviceManager() {
    checkConnectedDevices();  //initial check for available devices
}

AudioDeviceManager::~AudioDeviceManager() {
    closeOpenDevice();
}

/**
 * @brief Checks for connected recording audio devices and updates the available devices list.
 *
 * @return True if the currently open device remains unchanged, False if it was removed or specs changed.
 *         Returns always True if no device is open.
 */
bool AudioDeviceManager::checkConnectedDevices() {

    SDL_AudioDeviceID tempLogicalID = openDeviceID_ ? connectedDevices_[*openDeviceID_].logicalID : 0;
    SDL_AudioSpec tempSpec = openDeviceID_ ? connectedDevices_[*openDeviceID_].audioSpec : SDL_AudioSpec{};
    connectedDevices_.clear();
    
    SDL_AudioDeviceID* rawDeviceArray = nullptr;
    int numDevices = 0;
    rawDeviceArray = SDL_GetAudioRecordingDevices(&numDevices);
    
    //just rebuild entire device map to avoid error accumulation in possible edge cases, e.g. incomplete device event queue 
    if(rawDeviceArray != nullptr && numDevices > 0) {
        for (int i = 0; i < numDevices; i++) {

            SDL_AudioSpec spec{};
            const char* name = SDL_GetAudioDeviceName(rawDeviceArray[i]);
            bool formatSuccess = SDL_GetAudioDeviceFormat(rawDeviceArray[i], &spec, nullptr);

            if(name == nullptr || !formatSuccess) {
                SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Couldn't get name or format for audio device #%d: %s", i, SDL_GetError());
                continue;
            }

            connectedDevices_.emplace(rawDeviceArray[i], AudioDeviceInfo{name, rawDeviceArray[i], 0, spec});
            SDL_Log("Found recording device #%d (Hardware-ID: %d): '%s'", i, rawDeviceArray[i], name);
        }
    }
    
    SDL_free(rawDeviceArray);   //allocated by SDL, must be freed manually
    connectedDevices_.emplace(0, AudioDeviceInfo{"No Device", 0, 0, SDL_AudioSpec{}}); //add default device as last entry
    
    bool openDeviceUnchanged = true;

    //restore logical ID and check for changes of open device
    if(openDeviceID_) {
        //check for removal of open device
        if(connectedDevices_.contains(*openDeviceID_)) {
            connectedDevices_.at(*openDeviceID_).logicalID = tempLogicalID;
            //check for spec changes of open device
            if(connectedDevices_.at(*openDeviceID_).audioSpec.format != tempSpec.format ||
               connectedDevices_.at(*openDeviceID_).audioSpec.channels != tempSpec.channels ||
               connectedDevices_.at(*openDeviceID_).audioSpec.freq != tempSpec.freq) {
                openDeviceUnchanged = false;
            }

        } else {
            openDeviceUnchanged = false;
            openDeviceID_.reset();
        }
    }

    return openDeviceUnchanged;  //used for closing audiostreams if bound device was changed or removed
}

/**
 * @brief Opens the specified audio device for recording. Does nothing if device is already open.
 *
 * @param hardwareID The hardware ID of the audio device to open.
 * @return True if the device was successfully opened, False otherwise.
 */
bool AudioDeviceManager::openDevice(SDL_AudioDeviceID hardwareID) {

    if(openDeviceID_ && *openDeviceID_ == hardwareID) {
        return true;    //device already open
    }

    closeOpenDevice();   //just one open device at a time
    if(connectedDevices_.empty() || hardwareID == 0) { return false; }

    //open device
    SDL_AudioDeviceID logicalID = 0;
    logicalID = SDL_OpenAudioDevice(hardwareID, nullptr);
    if(logicalID != 0) {
        connectedDevices_[hardwareID].logicalID = logicalID;
        openDeviceID_ = hardwareID;

        std::cout << "Opened audio device: " << connectedDevices_[*openDeviceID_].name << std::endl;
        std::cout << "  Format: " << SDL_GetAudioFormatName(connectedDevices_[*openDeviceID_].audioSpec.format)
                  << " Channels: " << connectedDevices_[*openDeviceID_].audioSpec.channels
                  << " Frequency: " << connectedDevices_[*openDeviceID_].audioSpec.freq << std::endl;

    } else {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Couldn't open audio device: %s \n", SDL_GetError());
    }

    return logicalID != 0;
}

/**
 * @brief Closes the currently open audio device, if any. Does nothing if no device is open.
 * 
 */
void AudioDeviceManager::closeOpenDevice() {
    if(openDeviceID_) {
        SDL_CloseAudioDevice(connectedDevices_[*openDeviceID_].logicalID);
        connectedDevices_[*openDeviceID_].logicalID = 0;
        openDeviceID_.reset();
    }
}

/**
 * @brief Processes audio device changes like connection and disconnection of an audio devices at hardware level.
 * 
 * @return True if the currently open device remains unchanged, False if it was removed. Returns always True if no device is open.
 */
bool AudioDeviceManager::processAudioDeviceChanges() {
    SDL_Event audioDeviceEvent;
    bool openDeviceUnchanged = true; 

    SDL_PumpEvents(); //update event queue with latest events

    //filter just audio device events
	while (SDL_PeepEvents(&audioDeviceEvent, 1, SDL_GETEVENT, SDL_EVENT_AUDIO_DEVICE_ADDED, SDL_EVENT_AUDIO_DEVICE_FORMAT_CHANGED) > 0) {

        //filter just recording devices
        if(audioDeviceEvent.adevice.recording) {
            switch (audioDeviceEvent.type) {
                case SDL_EVENT_AUDIO_DEVICE_ADDED:
                    openDeviceUnchanged = checkConnectedDevices(); //refresh device list
                    SDL_Log("Audio device added: %s", SDL_GetAudioDeviceName(audioDeviceEvent.adevice.which));
                    break;
                case SDL_EVENT_AUDIO_DEVICE_REMOVED:
                    openDeviceUnchanged = checkConnectedDevices(); //refresh device list
                    SDL_Log("Audio device removed: %s", SDL_GetAudioDeviceName(audioDeviceEvent.adevice.which));
                    break;
                case SDL_EVENT_AUDIO_DEVICE_FORMAT_CHANGED:
                    openDeviceUnchanged = checkConnectedDevices(); //refresh device list
                    SDL_Log("Audio device format changed: %s", SDL_GetAudioDeviceName(audioDeviceEvent.adevice.which));
                    break;
                default:
                    break;
            }
        }
	}
    return openDeviceUnchanged;
}