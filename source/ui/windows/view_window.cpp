#include "view_window.h"

void ViewWindow::render(ApplicationState* appState) {
    ImGui::Begin("View");

    const float previewWidth = 320.0f;
    const float aspectRatio =
        static_cast<float>(appState->universalShaderSettings.windowHeight) / static_cast<float>(appState->universalShaderSettings.windowWidth);

    ImGui::Image(
        static_cast<ImTextureID>(appState->previewTexture),
        ImVec2(previewWidth, previewWidth * aspectRatio),
        ImVec2(0.0f, 1.0f),
        ImVec2(1.0f, 0.0f)
    );

    ImGui::End();
}