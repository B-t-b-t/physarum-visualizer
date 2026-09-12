#ifndef VIEW_WINDOW_H
#define VIEW_WINDOW_H

#include "base_window.h"
#include "../user_interface.h"

class ViewWindow : public BaseWindow {
public:
    void render(ApplicationState* appState) override;
};

#endif // VIEW_WINDOW_H