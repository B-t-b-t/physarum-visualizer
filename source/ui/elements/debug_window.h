#ifndef DEBUG_WINDOW_H
#define DEBUG_WINDOW_H

#include "imgui_window.h"
#include "../user_interface.h"

struct DebugInfos {
    int numberOfDisplays = 0;
    const SDL_DisplayMode* displayMode = nullptr;
    SDL_Rect displayUsableBounds;
    
    std::string glVersion = "";
	int maxVertexTextureUnits = 0;
	int maxFragmentTextureUnits = 0;
	int maxCombinedTextureUnits = 0;
	int maxImageUnits = 0;

    std::string audioDriver = "";
};

class DebugWindow : public ImGuiWindow {

public:
    DebugWindow();
    void render(UIState &state) override;

    void setGuiIO(ImGuiIO *guiIO) { guiIO_ = guiIO; };

private:

    ImGuiIO *guiIO_;
    DebugInfos debugInfos_;

    void fillDebugInfos();
};

#endif // DEBUG_WINDOW_H