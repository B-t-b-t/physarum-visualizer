#ifndef PRESET_SYSTEM_H
#define PRESET_SYSTEM_H

#include <filesystem>            // for path
#include <map>                   // for map
#include <string>                // for string, basic_string

#include "./utility/observer.h"  // for Observer

class ApplicationState;


template<typename T>
class PresetSystem : public Observer{
public:

	PresetSystem() = default;
	PresetSystem(std::filesystem::path presetFilePath, ApplicationState* appState);
	~PresetSystem();

	void onNotify(const UserEvent event) override;
	
private:
	
    void createPreset(std::string presetName);
    void savePresetsToFile();
    void loadPresetsFromFile();

    std::map<std::string, T> presets;
	std::string usedBehaviorPresetName_;

	std::filesystem::path presetFilePath_;

	ApplicationState* appState_;

	bool timeOut_ = false;
};

#endif // PRESET_SYSTEM_H