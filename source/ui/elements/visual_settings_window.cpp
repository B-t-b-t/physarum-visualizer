#include "visual_settings_window.h"

#include "../ui_helpers.h"
#include "../../utility/event.h"

void VisualSettingsWindow::render(ApplicationState* appState) {
    if(!visible) { return; }

    ImGui::Begin("Visual Settings", &visible);

	ImGui::SliderFloat("Brightness Multiplier", &appState->fragmentShaderSettings.brightnessMultiplier, 0.0f, 10.0f);

	ImGui::Combo("Tone Mapping", (int*)&appState->fragmentShaderSettings.toneMappingMode, "Reinhard\0Exposure\0ACES\0");

	if(appState->fragmentShaderSettings.toneMappingMode == 1) {
		ImGui::SliderFloat("Exposure", &appState->fragmentShaderSettings.exposure, 0.0f, 5.0f);
	}
	ImGui::Separator();

	bool bloom = appState->fragmentShaderSettings.bloomEnabled != 0;
	if (ImGui::Checkbox("Bloom Effect", &bloom)) {
		appState->fragmentShaderSettings.bloomEnabled = bloom ? 1 : 0;
	}
	if (bloom) {
		ImGui::SliderFloat("Bloom Intensity", &appState->fragmentShaderSettings.bloomIntensity, 0.0f, 10.0f);
		ImGui::SliderFloat("Bloom Threshold", &appState->fragmentShaderSettings.bloomThreshold, 0.0f, 1.0f);
		ImGui::SameLine(); HelpMarker("Threshold how bright areas have to be to be considered for bloom.");
		ImGui::SliderFloat("Bloom Knee", &appState->fragmentShaderSettings.bloomKnee, 0.0f, 1.0f);
		ImGui::Combo("Bloom Blend Mode", (int*)&appState->fragmentShaderSettings.bloomBlendMode, "Additive\0Screen\0Soft Additive\0");
		ImGui::Checkbox("Bloom Audio Reaction", &appState->bloomAudioReaction);
		if(appState->bloomAudioReaction) {
			ImGui::SliderFloat("Bloom Bass Reaction Intensity", &appState->bloomBassReactionIntensity, 0.0f, 5.0f);
		}
		ImGui::Separator();
	}
	
	// ImGui::Checkbox expects a bool*, but vignetteEffect is stored as an int in ApplicationState;
	// use a temporary bool to interface with ImGui and write back the result to the int.
	bool vignette = appState->fragmentShaderSettings.vignetteEffect != 0;
	if (ImGui::Checkbox("Vignette Effect", &vignette)) {
		appState->fragmentShaderSettings.vignetteEffect = vignette ? 1 : 0;
	}
	ImGui::SameLine(); HelpMarker("When a square render is not desired, useful for example for projecting the image with a beamer onto a wall. Different shapes possible from a circle to very elongated almost straight ellipses.");

	if(appState->fragmentShaderSettings.vignetteEffect) {
		ImGui::SliderInt("Vignette Selector", &appState->fragmentShaderSettings.vignetteSelector, 0, 1);
		ImGui::SliderFloat("Vignette Sharpness", &appState->fragmentShaderSettings.vignetteSharpness, 1.0f, 10.0f);
		ImGui::SliderFloat("Vignette Inner Radius", &appState->fragmentShaderSettings.vignetteInnerRadius, 0.5f, 10.0f);
		ImGui::SliderFloat("Vignette X Dimension", &appState->fragmentShaderSettings.vignetteXDimension, 0.0f, 10.0f);
		ImGui::SliderFloat("Vignette Y Dimension", &appState->fragmentShaderSettings.vignetteYDimension, 0.0f, 10.0f);
		ImGui::Separator();
	}
	if(ImGui::Checkbox("Fullscreen", &appState->fullscreen)) {
		notify(Event::FULLSCREEN_TOGGLE);
	}

	ImGui::End();
}