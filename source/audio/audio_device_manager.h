#ifndef AUDIO_DEVICE_MANAGER_H
#define AUDIO_DEVICE_MANAGER_H

#include <string>
#include <vector>
#include <optional>

#include <SDL3/SDL.h>

struct AudioDeviceInfo {
    const char* name;
    SDL_AudioDeviceID hardwareID = 0;
    SDL_AudioDeviceID logicalID = 0;    //0 for unbound, otherwise bound and in use
    SDL_AudioSpec currentAudioSpec_;
};

class AudioDeviceManager {
public:
    AudioDeviceManager();
    ~AudioDeviceManager();

    bool checkForAvailableDevices();

    bool openDevice(const std::string& deviceName);
    void closeCurrentDevice();
    bool processAudioDeviceEvents();

    SDL_AudioDeviceID getCurrentLogicalDeviceID() const { return usedDeviceIndex_ ? availableDevices_[*usedDeviceIndex_].logicalID : 0; }
    SDL_AudioSpec getCurrentAudioSpec() const { return usedDeviceIndex_ ? availableDevices_[*usedDeviceIndex_].currentAudioSpec_ : SDL_AudioSpec{}; }
    int getSampleRate() const { return usedDeviceIndex_ ? availableDevices_[*usedDeviceIndex_].currentAudioSpec_.freq : 0; }

    std::vector<AudioDeviceInfo>* getAvailableDevices() { return &availableDevices_; }
    std::vector<std::string> getAvailableDeviceNames();
    int getNumAvailableDevices() const { return availableDevices_.size(); }
    bool hasDevices() const { return !availableDevices_.empty(); }

private:
    std::vector<AudioDeviceInfo> availableDevices_;
    std::optional<size_t> usedDeviceIndex_{};

    std::optional<size_t> findDeviceIndex(const std::string& deviceName) const;
};

#endif // AUDIO_DEVICE_MANAGER_H