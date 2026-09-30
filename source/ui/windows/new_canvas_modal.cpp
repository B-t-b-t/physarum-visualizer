#include "new_canvas_modal.h"

#include <stddef.h>                   // for NULL
#include <string>                     // for basic_string

#include "../../application_state.h"  // for ApplicationState
#include "../../uniforms.h"           // for SlimeSettings, UniversalShaderS...
#include "../../utility/event.h"      // for EventType, UserEvent
#include "imgui.h"                    // for ImVec2, Separator, SliderFloat

void NewCanvasModal::render(ApplicationState* appState) {
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

		ImGui::PushItemWidth(widgetWidth);

		ImGui::Text("Create a new Canvas?\nThis operation cannot be undone!");
		ImGui::Separator();

		static int newTextureWidth{};
		static int newTextureHeight{};
		static int newNumParticles{};

		if(ImGui::IsWindowAppearing()) {
			newTextureWidth = appState->universalShaderSettings.textureWidth;
			newTextureHeight = appState->universalShaderSettings.textureHeight;
			newNumParticles = appState->numParticles;
		}

		ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0, 0));
		ImGui::InputInt("New Texture Width", &newTextureWidth, 8, 8, ImGuiInputTextFlags_CharsNoBlank);
		ImGui::SameLine();
		if (ImGui::SmallButton("/2##newWidth")) {
			newTextureWidth = static_cast<int>(newTextureWidth / 2.0f);
		}
		ImGui::SameLine();
		if (ImGui::SmallButton("/1.5##newWidth")) {
			newTextureWidth = static_cast<int>(newTextureWidth / 1.5f);
		}
		ImGui::SameLine();
		if (ImGui::SmallButton("x1.5##newWidth")) {
			newTextureWidth = static_cast<int>(newTextureWidth * 1.5f);
		}
		ImGui::SameLine();
		if (ImGui::SmallButton("x2##newWidth")) {
			newTextureWidth *= 2;
		}

		newTextureWidth = newTextureWidth - (newTextureWidth % 8);

		ImGui::InputInt("New Texture Height", &newTextureHeight, 8, 8, ImGuiInputTextFlags_CharsNoBlank);
				ImGui::SameLine();
		if (ImGui::SmallButton("/2##newHeight")) {
			newTextureHeight = static_cast<int>(newTextureHeight / 2.0f);
		}
		ImGui::SameLine();
		if (ImGui::SmallButton("/1.5##newHeight")) {
			newTextureHeight = static_cast<int>(newTextureHeight / 1.5f);
		}
		ImGui::SameLine();
		if (ImGui::SmallButton("x1.5##newHeight")) {
			newTextureHeight = static_cast<int>(newTextureHeight * 1.5f);
		}
		ImGui::SameLine();
		if (ImGui::SmallButton("x2##newHeight")) {
			newTextureHeight *= 2;
		}

		newTextureHeight = newTextureHeight - (newTextureHeight % 8);
		newNumParticles = appState->slimeRatio * newTextureWidth * newTextureHeight;
		ImGui::InputInt("Number of Particles", &newNumParticles, 8, 8, ImGuiInputTextFlags_CharsNoBlank);
		newNumParticles = newNumParticles - (newNumParticles % 8);
		appState->slimeRatio = newNumParticles / (float) (newTextureWidth * newTextureHeight);
		ImGui::InputFloat("Slime Ratio", &(appState->slimeRatio));
		ImGui::PopStyleVar();
		ImGui::Separator();

		ImGui::SliderFloat("Velocity", &(appState->slimeSettings.v), 0.0f, 3.0f);
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

		if (ImGui::Button("OK", ImVec2(120, 0))) {
			visible = false;
			
			if(newTextureWidth != appState->universalShaderSettings.textureWidth 
				|| newTextureHeight != appState->universalShaderSettings.textureHeight 
				|| newNumParticles != appState->numParticles) 
			{
				notify(UserEvent{EventType::NEW_CANVAS, NewCanvasData{newTextureWidth, newTextureHeight, newNumParticles}});
			}
			//textureWidth and Height have to be updated after notify, else the program state would be inconsistent
			appState->universalShaderSettings.textureWidth = newTextureWidth;
			appState->universalShaderSettings.textureHeight = newTextureHeight;
			appState->numParticles = newNumParticles;
			ImGui::CloseCurrentPopup();
		}

		ImGui::SetItemDefaultFocus();
		ImGui::SameLine();
		if (ImGui::Button("Cancel", ImVec2(120, 0))) { visible = false; ImGui::CloseCurrentPopup(); }

		ImGui::PopItemWidth();

		ImGui::EndPopup();
	}
}