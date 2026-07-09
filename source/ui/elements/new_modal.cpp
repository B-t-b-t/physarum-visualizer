#include "new_modal.h"

#include <iostream>

void NewModal::render(ApplicationState* appState) {
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
		ImGui::InputInt("New Texture Width", &(appState->newTextureWidth), 8, 8, ImGuiInputTextFlags_CharsNoBlank);
		appState->newTextureWidth = appState->newTextureWidth - (appState->newTextureWidth % 8);
		ImGui::InputInt("New Texture Height", &(appState->newTextureHeight), 8, 8, ImGuiInputTextFlags_CharsNoBlank);
		appState->newTextureHeight = appState->newTextureHeight - (appState->newTextureHeight % 8);
		appState->newNumParticles = appState->slimeRatio * appState->newTextureWidth * appState->newTextureHeight;
		ImGui::InputInt("Number of Particles", &(appState->newNumParticles), 8, 8, ImGuiInputTextFlags_CharsNoBlank);
		appState->newNumParticles = appState->newNumParticles - (appState->newNumParticles % 8);
		appState->slimeRatio = appState->newNumParticles / (float) (appState->newTextureWidth * appState->newTextureHeight);
		ImGui::InputFloat("Slime Ratio", &(appState->slimeRatio));
		ImGui::PopStyleVar();
		ImGui::Separator();

		ImGui::SliderFloat("v", &(appState->slimeSettings.v), 0.0f, 3.0f);
		ImGui::SliderInt("Rotation Angle", &(appState->slimeSettings.rotationAngle), 0, 180);
		ImGui::SliderInt("Sensor Angle", &(appState->slimeSettings.angle), 0, 180);
		
		ImGui::SliderInt("Sensor Distance", &(appState->slimeSettings.sensorDistance), 1, 100);
		ImGui::SliderFloat("Deposition Strength", &(appState->slimeSettings.depositionStrength), 0.0f, 10.0f);
		ImGui::SliderFloat("diffusionWeight", &(appState->trailDiffusionSettings.diffusionWeight), 0.0f, 1.0f);
		ImGui::SliderFloat("decay", &(appState->trailDiffusionSettings.decay), 0.0f, 1.0f);

		ImGui::Separator();
		
		ImGui::ColorEdit3("Slime Color 0", (float*)&(appState->slimeSettings.slimeColor0));
		ImGui::ColorEdit3("Slime Color 1", (float*)&(appState->slimeSettings.slimeColor1));
		ImGui::ColorEdit3("Slime Color 2", (float*)&(appState->slimeSettings.slimeColor2));
		
		ImGui::Separator();
		
		ImGui::Checkbox("Use Particle Mask instead of Color", (bool*)&(appState->slimeSettings.useMask));
		
		ImGui::Checkbox("Collision Detection", (bool*)&(appState->universalShaderSettings.collisionDetection));

		if (ImGui::Button("OK", ImVec2(120, 0))) {  visible = false;
                                                    notify(Event::NEW_CANVAS);
                                                    //new canvas has been created, update the appState accordingly
                                                    appState->universalShaderSettings.textureWidth = appState->newTextureWidth;
                                                    appState->universalShaderSettings.textureHeight = appState->newTextureHeight;
                                                    appState->numParticles = appState->newNumParticles;
                                                    std::cout << "Creating new Canvas with " << appState->numParticles << " particles and size " << appState->universalShaderSettings.textureWidth << "x" << appState->universalShaderSettings.textureHeight << std::endl;                                   
                                                    ImGui::CloseCurrentPopup(); }
		ImGui::SetItemDefaultFocus();
		ImGui::SameLine();
		if (ImGui::Button("Cancel", ImVec2(120, 0))) { visible = false; ImGui::CloseCurrentPopup(); }
		ImGui::EndPopup();
	}
}