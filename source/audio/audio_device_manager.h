#ifndef AUDIO_DEVICE_MANAGER_H
#define AUDIO_DEVICE_MANAGER_H

#include <string>
#include <vector>
#include <optional>

#include <SDL3/SDL.h>

struct AudioDeviceInfo {
    const char* name;
    SDL_AudioDeviceID hardwareID = 0;
};

class AudioDeviceManager {
public:
    AudioDeviceManager();
    ~AudioDeviceManager();

    void checkForAvailableDevices();

    bool openDevice(const std::string& deviceName);
    void closeCurrentDevice();

    SDL_AudioDeviceID getCurrentLogicalDeviceID() const { return currentLogicalID_; }
    SDL_AudioSpec getCurrentAudioSpec() const { return currentAudioSpec_; }
    int getSampleRate() const { return currentAudioSpec_.freq; }

    std::vector<std::string> getAvailableDeviceNames() const;
    int getNumAvailableDevices() const { return availableDevices_.size(); }
    bool hasDevices() const { return !availableDevices_.empty(); }

private:
    std::vector<AudioDeviceInfo> availableDevices_;
    SDL_AudioDeviceID currentLogicalID_ = 0;
    SDL_AudioSpec currentAudioSpec_;

    std::optional<size_t> findDeviceIndex(const std::string& deviceName) const;
};

#endif // AUDIO_DEVICE_MANAGER_H