#include "debug_window.h"

#include <GL/glew.h>

DebugWindow::DebugWindow() {
	fillDebugInfos();
}

void DebugWindow::render(ApplicationState* state) {
    if(!visible) { return; }

	ImGui::Begin("Debug", &visible);

	ImGui::Checkbox("Render Particles", (bool*)&state->universalShaderSettings.renderParticles);
	ImGui::Checkbox("Lock Particle Color to Color 0", &state->lockParticleColor);
	ImGui::ColorEdit3("Particle Color 0", (float*)&state->slimeSettings.particleColor0);
	ImGui::ColorEdit3("Particle Color 1", (float*)&state->slimeSettings.particleColor1);
	ImGui::ColorEdit3("Particle Color 2", (float*)&state->slimeSettings.particleColor2);

	ImGui::Separator();

	ImGui::Checkbox("Render Collisions", (bool*)&state->universalShaderSettings.renderCollisions);
	if(state->universalShaderSettings.renderCollisions) {
		ImGui::ColorEdit3("Collision Color", (float*)&state->slimeSettings.collisionColor);
	}

	ImGui::Separator();

	// ImGui::Checkbox expects a bool*, but renderColorTraces is stored as an int in ApplicationState;
	// use a temporary bool to interface with ImGui and write back the result to the int.
	bool renderColorTraces = state->fragmentShaderSettings.renderColorTraces != 0;
	if (ImGui::Checkbox("Render Color Traces", &renderColorTraces)) {
		state->fragmentShaderSettings.renderColorTraces = renderColorTraces ? 1 : 0;
	}

	ImGui::ColorEdit4("Clear Color", (float*)&state->clearColor);

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
            const bool is_selected = (state->fragmentShaderSettings.debugTextureMaskSelector == n);
            if (ImGui::Selectable(textureMaskNames[n], is_selected)) {
                state->fragmentShaderSettings.debugTextureMaskSelector = n;
                state->selectedTextureMask = static_cast<TextureMask>(n);
            }
            
            // Set the initial focus when opening the combo (scrolling + keyboard navigation focus)
            if (is_selected) {
                ImGui::SetItemDefaultFocus();
            }
        }

        ImGui::EndListBox();
    }

	ImGui::SeparatorText("Infos");
	ImGui::Text("Number of Particles: %d", state->numParticles);
	ImGui::Text("WindowWidth: %d TextureWidth: %d NewTextureWidth: %d Height: %d TextureHeight: %d NewTextureHeight: %d", state->universalShaderSettings.windowWidth, state->universalShaderSettings.textureWidth, state->newTextureWidth, state->universalShaderSettings.windowHeight, state->universalShaderSettings.textureHeight, state->newTextureHeight);
	ImGui::Text("Fullscreen: %d", state->fullscreen);

	if(ImGui::CollapsingHeader("Display Infos")) {
		ImGui::Text("Number of Displays: %d", debugInfos_.numberOfDisplays);
		if (debugInfos_.displayMode) {
			ImGui::Text("Current Display ID: %d", debugInfos_.displayMode->displayID);
			ImGui::Text("Real Display Resolution: %dx%d @ %fHz", (int)(debugInfos_.displayMode->w * debugInfos_.displayMode->pixel_density), (int)(debugInfos_.displayMode->h * debugInfos_.displayMode->pixel_density), debugInfos_.displayMode->refresh_rate);
			ImGui::Text("Scaled Display Resolution: %dx%d at Scaling Factor %.2f", debugInfos_.displayMode->w, debugInfos_.displayMode->h, (float)debugInfos_.displayMode->pixel_density);
			ImGui::Text("Usable Display Area: %dx%d at (%d, %d)", debugInfos_.displayUsableBounds.w, debugInfos_.displayUsableBounds.h, debugInfos_.displayUsableBounds.x, debugInfos_.displayUsableBounds.y);
			ImGui::Text("Display Format: %s", SDL_GetPixelFormatName(debugInfos_.displayMode->format));
		}
	}

	if (ImGui::CollapsingHeader("OpenGL Capabilities")) {
		ImGui::Text("OpenGL Version: %s", debugInfos_.glVersion.c_str());

		ImGui::Text("Max Vertex Texture Image Units: %d", debugInfos_.maxVertexTextureUnits);
		ImGui::Text("Max Fragment Texture Image Units: %d", debugInfos_.maxFragmentTextureUnits);
		ImGui::Text("Max Combined Texture Image Units: %d", debugInfos_.maxCombinedTextureUnits);
		ImGui::Text("Max Image Units: %d", debugInfos_.maxImageUnits);
	}

	if (ImGui::CollapsingHeader("Audio Infos")) {
		ImGui::Text("Current Audio Driver: %s", debugInfos_.audioDriver.c_str());
	}

	if (ImGui::CollapsingHeader("Mouse Input")) {
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
	}
	
	ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);

	ImGui::End();

	if (state->lockParticleColor) {
		state->slimeSettings.particleColor1 = state->slimeSettings.particleColor0;
		state->slimeSettings.particleColor2 = state->slimeSettings.particleColor0;
	}
}

void DebugWindow::fillDebugInfos() {
	debugInfos_.glVersion = reinterpret_cast<const char*>(glGetString(GL_VERSION));
	
	SDL_DisplayID* displays = SDL_GetDisplays(&debugInfos_.numberOfDisplays);

	debugInfos_.displayMode = SDL_GetCurrentDisplayMode(*displays);

	bool displayBoundSuccess = SDL_GetDisplayUsableBounds(*displays, &debugInfos_.displayUsableBounds);
	if(!displayBoundSuccess) {
		debugInfos_.displayUsableBounds = {0, 0, 0, 0};
	}

	glGetIntegerv(GL_MAX_VERTEX_TEXTURE_IMAGE_UNITS, &debugInfos_.maxVertexTextureUnits);
	glGetIntegerv(GL_MAX_TEXTURE_IMAGE_UNITS, &debugInfos_.maxFragmentTextureUnits);
	glGetIntegerv(GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS, &debugInfos_.maxCombinedTextureUnits);
	glGetIntegerv(GL_MAX_IMAGE_UNITS, &debugInfos_.maxImageUnits);

	debugInfos_.audioDriver = SDL_GetCurrentAudioDriver();
}