#include "visual_settings_window.h"

#include "../ui_helpers.h"

void VisualSettingsWindow::render(UIState &state) {
    if(!visible) { return; }

    ImGui::Begin("Visual Settings", &visible);

	ImGui::SliderFloat("Brightness Multiplier", &state.fragmentShaderSettings.brightnessMultiplier, 0.0f, 10.0f);

	ImGui::Combo("Tone Mapping", (int*)&state.fragmentShaderSettings.toneMappingMode, "Reinhard\0Exposure\0ACES\0");

	if(state.fragmentShaderSettings.toneMappingMode == 1) {
		ImGui::SliderFloat("Exposure", &state.fragmentShaderSettings.exposure, 0.0f, 5.0f);
	}
	ImGui::Separator();

	bool bloom = state.fragmentShaderSettings.bloomEnabled != 0;
	if (ImGui::Checkbox("Bloom Effect", &bloom)) {
		state.fragmentShaderSettings.bloomEnabled = bloom ? 1 : 0;
	}
	if (bloom) {
		ImGui::SliderFloat("Bloom Intensity", &state.fragmentShaderSettings.bloomIntensity, 0.0f, 10.0f);
		ImGui::SliderFloat("Bloom Threshold", &state.fragmentShaderSettings.bloomThreshold, 0.0f, 1.0f);
		ImGui::SameLine(); HelpMarker("Threshold how bright areas have to be to be considered for bloom.");
		ImGui::SliderFloat("Bloom Knee", &state.fragmentShaderSettings.bloomKnee, 0.0f, 1.0f);
		ImGui::Combo("Bloom Blend Mode", (int*)&state.fragmentShaderSettings.bloomBlendMode, "Additive\0Screen\0Soft Additive\0");
		ImGui::Checkbox("Bloom Audio Reaction", &state.bloomAudioReaction);
		if(state.bloomAudioReaction) {
			ImGui::SliderFloat("Bloom Bass Reaction Intensity", &state.bloomBassReactionIntensity, 0.0f, 5.0f);
		}
		ImGui::Separator();
	}
	
	// ImGui::Checkbox expects a bool*, but vignetteEffect is stored as an int in UIState;
	// use a temporary bool to interface with ImGui and write back the result to the int.
	bool vignette = state.fragmentShaderSettings.vignetteEffect != 0;
	if (ImGui::Checkbox("Vignette Effect", &vignette)) {
		state.fragmentShaderSettings.vignetteEffect = vignette ? 1 : 0;
	}
	ImGui::SameLine(); HelpMarker("When a square render is not desired, useful for example for projecting the image with a beamer onto a wall. Different shapes possible from a circle to very elongated almost straight ellipses.");

	if(state.fragmentShaderSettings.vignetteEffect) {
		ImGui::SliderInt("Vignette Selector", &state.fragmentShaderSettings.vignetteSelector, 0, 1);
		ImGui::SliderFloat("Vignette Sharpness", &state.fragmentShaderSettings.vignetteSharpness, 1.0f, 10.0f);
		ImGui::SliderFloat("Vignette Inner Radius", &state.fragmentShaderSettings.vignetteInnerRadius, 0.5f, 10.0f);
		ImGui::SliderFloat("Vignette X Dimension", &state.fragmentShaderSettings.vignetteXDimension, 0.0f, 10.0f);
		ImGui::SliderFloat("Vignette Y Dimension", &state.fragmentShaderSettings.vignetteYDimension, 0.0f, 10.0f);
		ImGui::Separator();
	}
	ImGui::Checkbox("Fullscreen", &state.fullscreen);

	ImGui::End();
}