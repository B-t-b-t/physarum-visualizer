#ifndef SLIME_CONFIG_WINDOW_H
#define SLIME_CONFIG_WINDOW_H

#include "base_window.h"
#include "../user_interface.h"

class SlimeConfigWindow : public BaseWindow {

public:
    void render(ApplicationState* appState) override;
};

#endif // SLIME_CONFIG_WINDOW_H