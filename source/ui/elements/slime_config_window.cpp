#include "slime_config_window.h"

void SlimeConfigWindow::render(UIState* state) {
    if(!visible) { return; }

    ImGui::Begin("Slime Config", &visible);
	
	ImGui::SliderFloat("v", &state->slimeSettings.v, 0.0f, 3.0f);
	ImGui::SliderInt("Rotation Angle", &state->slimeSettings.rotationAngle, 0, 180);
	ImGui::SliderInt("Sensor Angle", &state->slimeSettings.angle, 0, 180);
	
	ImGui::Checkbox("Lock Angles", &state->lockAngles);
	ImGui::SliderInt("Sensor Distance", &state->slimeSettings.sensorDistance, 1, 100);
	ImGui::SliderFloat("Deposition Strength", &state->slimeSettings.depositionStrength, 0.0f, 10.0f);
	ImGui::SliderFloat("diffusionWeight", &state->trailDiffusionSettings.diffusionWeight, 0.0f, 1.0f);
	ImGui::SliderFloat("decay", &state->trailDiffusionSettings.decay, 0.0f, 1.0f);

	ImGui::Separator();
	
	ImGui::Checkbox("Lock Slime Color to Color 0", &state->lockSlimeColor);
	
	ImGui::ColorEdit3("Slime Color 0", (float*)&state->slimeSettings.slimeColor0);
	ImGui::ColorEdit3("Slime Color 1", (float*)&state->slimeSettings.slimeColor1);
	ImGui::ColorEdit3("Slime Color 2", (float*)&state->slimeSettings.slimeColor2);
	
	ImGui::Separator();
	
	ImGui::Checkbox("Use Particle Mask instead of Color", (bool*)&state->slimeSettings.useMask);
	
	ImGui::Checkbox("Collision Detection", (bool*)&state->universalShaderSettings.collisionDetection);
	if(state->universalShaderSettings.collisionDetection) {
		ImGui::SliderInt("Density Limit", (int*)&state->slimeSettings.densityLimit, 1, 20);
	}

	ImGui::Checkbox("Enable Parameters", (bool*)&state->parameterSettings.enableParameters);
	if(state->parameterSettings.enableParameters) {

		ImGui::Separator();
		ImGui::Text("Sensor Distance");
		ImGui::SliderFloat("p1", &state->parameterSettings.p1, 0.0f, 30.0f);
		ImGui::SliderFloat("p2", &state->parameterSettings.p2, 0.0f, 30.0f);
		ImGui::SliderFloat("p3", &state->parameterSettings.p3, 0.0f, 30.0f);

		ImGui::Separator();
		ImGui::Text("Sensor Angle");
		ImGui::SliderFloat("p4", &state->parameterSettings.p4, 0.0f, 30.0f);
		ImGui::SliderFloat("p5", &state->parameterSettings.p5, 0.0f, 30.0f);
		ImGui::SliderFloat("p6", &state->parameterSettings.p6, 0.0f, 30.0f);

		ImGui::Separator();
		ImGui::Text("Rotation Angle");
		ImGui::SliderFloat("p7", &state->parameterSettings.p7, 0.0f, 30.0f);
		ImGui::SliderFloat("p8", &state->parameterSettings.p8, 0.0f, 30.0f);
		ImGui::SliderFloat("p9", &state->parameterSettings.p9, 0.0f, 30.0f);

		ImGui::Separator();
		ImGui::Text("Move Distance");
		ImGui::SliderFloat("p10", &state->parameterSettings.p10, 0.0f, 30.0f);
		ImGui::SliderFloat("p11", &state->parameterSettings.p11, 0.0f, 30.0f);
		ImGui::SliderFloat("p12", &state->parameterSettings.p12, 0.0f, 30.0f);
	}

	ImGui::End();
	
	if (state->lockAngles) {
		state->slimeSettings.angle = state->slimeSettings.rotationAngle;
	}
	
	if (state->lockSlimeColor) {
		state->slimeSettings.slimeColor1 = state->slimeSettings.slimeColor0;
		state->slimeSettings.slimeColor2 = state->slimeSettings.slimeColor0;
	}
	
	if (state->lockParticleColor) {
		state->slimeSettings.particleColor1 = state->slimeSettings.particleColor0;
		state->slimeSettings.particleColor2 = state->slimeSettings.particleColor0;
	}
}