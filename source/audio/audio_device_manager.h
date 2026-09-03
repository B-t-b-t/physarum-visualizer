#ifndef AUDIO_DEVICE_MANAGER_H
#define AUDIO_DEVICE_MANAGER_H

#include <map>               // for map
#include <optional>          // for optional, nullopt, nullopt_t

#include "SDL3/SDL_audio.h"  // for SDL_AudioDeviceID, SDL_AudioSpec

struct AudioDeviceInfo {
    const char* name;
    SDL_AudioDeviceID hardwareID = 0;
    SDL_AudioDeviceID logicalID = 0;    //0 for unbound, otherwise bound and in use
    SDL_AudioSpec audioSpec;
};

class AudioDeviceManager {
public:
    AudioDeviceManager();
    ~AudioDeviceManager();

    bool checkConnectedDevices();

    bool openDevice(SDL_AudioDeviceID hardwareID);
    void closeOpenDevice();
    bool processAudioDeviceChanges();

    std::optional<AudioDeviceInfo> getOpenDevice() const { return openDeviceID_ ? std::optional<AudioDeviceInfo>(connectedDevices_.at(*openDeviceID_)) : std::nullopt; }
    std::map<SDL_AudioDeviceID, AudioDeviceInfo>* getAvailableDevices() { return &connectedDevices_; }

private:
    std::map<SDL_AudioDeviceID, AudioDeviceInfo> connectedDevices_;
    std::optional<SDL_AudioDeviceID> openDeviceID_{};
};

#endif // AUDIO_DEVICE_MANAGER_H