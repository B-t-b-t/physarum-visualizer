#ifndef VISUAL_SETTINGS_WINDOW_H
#define VISUAL_SETTINGS_WINDOW_H

#include "base_window.h"
#include "../user_interface.h"

class VisualSettingsWindow : public BaseWindow {

public:
    void render(ApplicationState* appState) override;
};

#endif // VISUAL_SETTINGS_WINDOW_H