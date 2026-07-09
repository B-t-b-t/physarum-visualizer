#include "new_modal.h"

#include <iostream>

void NewModal::render(ApplicationState* state) {
	if (visible) {
		ImGui::OpenPopup("New Canvas");
	} else {
        return;
    }

	// Always center this window when appearing
	ImVec2 center = ImGui::GetMainViewport()->GetCenter();
	ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));

    //BeginPopupModal returns true if Popup "New Canvas" is open
	if (ImGui::BeginPopupModal("New Canvas", NULL, ImGuiWindowFlags_AlwaysAutoResize)) {

		ImGui::Text("Create a new Canvas?\nThis operation cannot be undone!");
		ImGui::Separator();


		ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0, 0));
		ImGui::InputInt("New Texture Width", &(state->newTextureWidth), 8, 8, ImGuiInputTextFlags_CharsNoBlank);
		state->newTextureWidth = state->newTextureWidth - (state->newTextureWidth % 8);
		ImGui::InputInt("New Texture Height", &(state->newTextureHeight), 8, 8, ImGuiInputTextFlags_CharsNoBlank);
		state->newTextureHeight = state->newTextureHeight - (state->newTextureHeight % 8);
		state->newNumParticles = state->slimeRatio * state->newTextureWidth * state->newTextureHeight;
		ImGui::InputInt("Number of Particles", &(state->newNumParticles), 8, 8, ImGuiInputTextFlags_CharsNoBlank);
		state->newNumParticles = state->newNumParticles - (state->newNumParticles % 8);
		state->slimeRatio = state->newNumParticles / (float) (state->newTextureWidth * state->newTextureHeight);
		ImGui::InputFloat("Slime Ratio", &(state->slimeRatio));
		ImGui::PopStyleVar();
		ImGui::Separator();

		ImGui::SliderFloat("v", &(state->slimeSettings.v), 0.0f, 3.0f);
		ImGui::SliderInt("Rotation Angle", &(state->slimeSettings.rotationAngle), 0, 180);
		ImGui::SliderInt("Sensor Angle", &(state->slimeSettings.angle), 0, 180);
		
		ImGui::SliderInt("Sensor Distance", &(state->slimeSettings.sensorDistance), 1, 100);
		ImGui::SliderFloat("Deposition Strength", &(state->slimeSettings.depositionStrength), 0.0f, 10.0f);
		ImGui::SliderFloat("diffusionWeight", &(state->trailDiffusionSettings.diffusionWeight), 0.0f, 1.0f);
		ImGui::SliderFloat("decay", &(state->trailDiffusionSettings.decay), 0.0f, 1.0f);

		ImGui::Separator();
		
		ImGui::ColorEdit3("Slime Color 0", (float*)&(state->slimeSettings.slimeColor0));
		ImGui::ColorEdit3("Slime Color 1", (float*)&(state->slimeSettings.slimeColor1));
		ImGui::ColorEdit3("Slime Color 2", (float*)&(state->slimeSettings.slimeColor2));
		
		ImGui::Separator();
		
		ImGui::Checkbox("Use Particle Mask instead of Color", (bool*)&(state->slimeSettings.useMask));
		
		ImGui::Checkbox("Collision Detection", (bool*)&(state->universalShaderSettings.collisionDetection));

		if (ImGui::Button("OK", ImVec2(120, 0))) {  visible = false;
                                                    notify(Event::NEW_CANVAS);
                                                    //new canvas has been created, update the state accordingly
                                                    state->universalShaderSettings.textureWidth = state->newTextureWidth;
                                                    state->universalShaderSettings.textureHeight = state->newTextureHeight;
                                                    state->numParticles = state->newNumParticles;
                                                    std::cout << "Creating new Canvas with " << state->numParticles << " particles and size " << state->universalShaderSettings.textureWidth << "x" << state->universalShaderSettings.textureHeight << std::endl;                                   
                                                    ImGui::CloseCurrentPopup(); }
		ImGui::SetItemDefaultFocus();
		ImGui::SameLine();
		if (ImGui::Button("Cancel", ImVec2(120, 0))) { visible = false; ImGui::CloseCurrentPopup(); }
		ImGui::EndPopup();
	}
}