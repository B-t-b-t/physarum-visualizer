#include "visual_settings_window.h"

#include "link_element.h"
#include "../ui_helpers.h"
#include "../../utility/event.h"

void VisualSettingsWindow::render(ApplicationState* appState) {
    if(!visible) { return; }
	
    ImGui::Begin("Visual Settings", &visible);

	//ImGui::SliderFloat("Brightness Multiplier", &appState->fragmentShaderSettings.brightnessMultiplier, 0.0f, 10.0f);
	ImGui::SeparatorText("Tone Mapping");
	ImGui::Combo("##Tone Mapping", (int*)&appState->fragmentShaderSettings.toneMappingMode, "Reinhard\0Exposure\0ACES\0");

	if(appState->fragmentShaderSettings.toneMappingMode == 1) {
		ImGui::SliderFloat("Exposure", &appState->fragmentShaderSettings.exposure, 0.0f, 5.0f);
	}
	ImGui::SeparatorText("Bloom");

	bool bloom = appState->fragmentShaderSettings.bloomEnabled != 0;
	if (ImGui::Checkbox("Enable##Bloom", &bloom)) {
		appState->fragmentShaderSettings.bloomEnabled = bloom ? 1 : 0;
	}
	if (bloom) {
		ImGui::SliderFloat("Intensity", &appState->fragmentShaderSettings.bloomIntensity, 0.0f, 10.0f);
		ImGui::SliderFloat("Threshold", &appState->fragmentShaderSettings.bloomThreshold, 0.0f, 1.0f);
		ImGui::SameLine(); HelpMarker("Threshold: how bright areas have to be to be considered for bloom.");
		ImGui::SliderFloat("Knee", &appState->fragmentShaderSettings.bloomKnee, 0.0f, 1.0f);
		ImGui::Combo("Blend Mode", (int*)&appState->fragmentShaderSettings.bloomBlendMode, "Additive\0Screen\0Soft Additive\0");
		ImGui::Checkbox("Audio Reaction", &appState->bloomAudioReaction);
		if(appState->bloomAudioReaction) {
			ImGui::SliderFloat("Bass Reaction Intensity", &appState->bloomBassReactionIntensity, 0.0f, 5.0f);
		}
	}
	
	ImGui::SeparatorText("Vignette");
	// ImGui::Checkbox expects a bool*, but vignetteEffect is stored as an int in ApplicationState;
	// use a temporary bool to interface with ImGui and write back the result to the int.
	bool vignette = appState->fragmentShaderSettings.vignetteEffect != 0;
	if (ImGui::Checkbox("Enable##Vignette", &vignette)) {
		appState->fragmentShaderSettings.vignetteEffect = vignette ? 1 : 0;
	}
	ImGui::SameLine(); HelpMarker("When a square render is not desired, useful for example for projecting the image with a beamer onto a wall. Different shapes possible from a circle to very elongated almost straight ellipses.");

	if(appState->fragmentShaderSettings.vignetteEffect) {
		ImGui::SliderInt("Selector", &appState->fragmentShaderSettings.vignetteSelector, 0, 1);
		ImGui::SliderFloat("Sharpness", &appState->fragmentShaderSettings.vignetteSharpness, 1.0f, 10.0f);
		ImGui::SliderFloat("Inner Radius", &appState->fragmentShaderSettings.vignetteInnerRadius, 0.5f, 10.0f);

		ImGui::LinkBegin("##Link Vignette Dimensions", &lockVignetteDimensions_, 2.0f);

		ImGui::LinkSliderFloat("Width", &appState->fragmentShaderSettings.vignetteXDimension, 0.0f, 10.0f);
		ImGui::LinkSliderFloat("Height", &appState->fragmentShaderSettings.vignetteYDimension, 0.0f, 10.0f); 

		ImGui::LinkEnd();

		ImGui::Separator();
	}
	ImGui::SeparatorText("Display");
	if(ImGui::Checkbox("Fullscreen", &appState->fullscreen)) {
		notify(UserEvent{EventType::FULLSCREEN_TOGGLE, 0, 0});
	}

	ImGui::End();
}