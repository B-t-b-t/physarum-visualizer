#include "DebugWindow.h"

void DebugWindow::render(UIState& state) {
    if(!visible) { return; }

	ImGui::Begin("Debug", &visible);

	ImGui::Checkbox("Render Particles", (bool*)&state.universalShaderSettings.renderParticles);
	ImGui::Checkbox("Lock Particle Color to Color 0", &state.lockParticleColor);
	ImGui::ColorEdit3("Particle Color 0", (float*)&state.slimeSettings.particleColor0);
	ImGui::ColorEdit3("Particle Color 1", (float*)&state.slimeSettings.particleColor1);
	ImGui::ColorEdit3("Particle Color 2", (float*)&state.slimeSettings.particleColor2);

	ImGui::Separator();

	ImGui::Checkbox("Render Collisions", (bool*)&state.universalShaderSettings.renderCollisions);
	if(state.universalShaderSettings.renderCollisions) {
		ImGui::ColorEdit3("Collision Color", (float*)&state.slimeSettings.collisionColor);
	}

	ImGui::Separator();

	// ImGui::Checkbox expects a bool*, but renderColorTraces is stored as an int in UIState;
	// use a temporary bool to interface with ImGui and write back the result to the int.
	bool renderColorTraces = state.fragmentShaderSettings.renderColorTraces != 0;
	if (ImGui::Checkbox("Render Color Traces", &renderColorTraces)) {
		state.fragmentShaderSettings.renderColorTraces = renderColorTraces ? 1 : 0;
	}

	ImGui::ColorEdit4("Clear Color", (float*)&state.clearColor);

    if (ImGui::BeginListBox("Texture Mask")) {
        const char* textureMaskNames[] = {
            "Trail Texture",
            "Trail Non-Diffused",
            "New Particles",
            "Old Particles",
            "Collisions",
            "Bloom",
			"UpSample 1",
			"DownSample 2",
			"UpSample 2",
			"DownSample 3",
			"UpSample 3",
			"DownSample 4",
			"UpSample 4",
			"DownSample 5",
			"UpSample 5",
			"Threshold"
        };

        for (int n = 0; n < IM_ARRAYSIZE(textureMaskNames); n++) {
            const bool is_selected = (state.fragmentShaderSettings.debugTextureMaskSelector == n);
            if (ImGui::Selectable(textureMaskNames[n], is_selected)) {
                state.fragmentShaderSettings.debugTextureMaskSelector = n;
                state.selectedTextureMask = static_cast<TextureMask>(n);
            }
            
            // Set the initial focus when opening the combo (scrolling + keyboard navigation focus)
            if (is_selected) {
                ImGui::SetItemDefaultFocus();
            }
        }

        ImGui::EndListBox();
    }

	ImGui::SeparatorText("Infos");
	ImGui::Text("Number of Particles: %d", state.numParticles);
	ImGui::Text("WindowWidth: %d TextureWidth: %d NewTextureWidth: %d Height: %d TextureHeight: %d NewTextureHeight: %d", state.universalShaderSettings.windowWidth, state.universalShaderSettings.textureWidth, state.newTextureWidth, state.universalShaderSettings.windowHeight, state.universalShaderSettings.textureHeight, state.newTextureHeight);
	ImGui::Text("Fullscreen: %d", state.fullscreen);

	if (ImGui::IsMousePosValid()) {
		ImGui::Text("Mouse pos: (%g, %g)", guiIO_->MousePos.x, guiIO_->MousePos.y);
	}
	else{
		ImGui::Text("Mouse pos: <INVALID>");
	}
	ImGui::Text("Mouse delta: (%g, %g)", guiIO_->MouseDelta.x, guiIO_->MouseDelta.y);
	ImGui::Text("Mouse down:");
	for (int i = 0; i < IM_ARRAYSIZE(guiIO_->MouseDown); i++) {
		if (ImGui::IsMouseDown(i)) { 
			ImGui::SameLine(); ImGui::Text("b%d (%.02f secs)", i, guiIO_->MouseDownDuration[i]); 
		}
	}
	ImGui::Text("Mouse wheel: %.1f", guiIO_->MouseWheel);
	
	ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);

	ImGui::End();

	if (state.lockParticleColor) {
		state.slimeSettings.particleColor1 = state.slimeSettings.particleColor0;
		state.slimeSettings.particleColor2 = state.slimeSettings.particleColor0;
	}
}