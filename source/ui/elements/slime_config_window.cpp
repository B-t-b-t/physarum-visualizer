#include "slime_config_window.h"

#include "link_element.h"

void SlimeConfigWindow::render(ApplicationState* appState) {
    if(!visible) { return; }

    ImGui::Begin("Slime Config", &visible);
	
	ImGui::SliderFloat("v", &appState->slimeSettings.v, 0.0f, 3.0f);

	ImGui::LinkBegin("##Link Angles", &appState->lockAngles, 2.0f);

	ImGui::LinkSliderInt("Rotation Angle", &appState->slimeSettings.rotationAngle, 0, 180);
	ImGui::LinkSliderInt("Sensing Angle ", &appState->slimeSettings.angle, 0, 180);

	ImGui::LinkEnd();
	
	ImGui::SliderInt("Sensor Distance", &appState->slimeSettings.sensorDistance, 1, 100);
	ImGui::SliderFloat("Deposition Strength", &appState->slimeSettings.depositionStrength, 0.0f, 10.0f);
	ImGui::SliderFloat("diffusionWeight", &appState->trailDiffusionSettings.diffusionWeight, 0.0f, 1.0f);
	ImGui::SliderFloat("decay", &appState->trailDiffusionSettings.decay, 0.0f, 1.0f);

	ImGui::Separator();
	
	ImGui::LinkBegin("##Link Slime Colors", &appState->lockSlimeColor, 2.0f);

	ImGui::LinkColorEdit3("Color Species 0", (float*)&appState->slimeSettings.slimeColor0);
	ImGui::LinkColorEdit3("Color Species 1", (float*)&appState->slimeSettings.slimeColor1);
	ImGui::LinkColorEdit3("Color Species 2", (float*)&appState->slimeSettings.slimeColor2);

	ImGui::LinkEnd();

	ImGui::Separator();
	
	ImGui::Checkbox("Use Particle Mask instead of Color", (bool*)&appState->slimeSettings.useMask);
	
	ImGui::Checkbox("Collision Detection", (bool*)&appState->universalShaderSettings.collisionDetection);
	if(appState->universalShaderSettings.collisionDetection) {
		ImGui::SliderInt("Density Limit", (int*)&appState->slimeSettings.densityLimit, 1, 20);
	}

	ImGui::Checkbox("Enable Parameters", (bool*)&appState->parameterSettings.enableParameters);
	if(appState->parameterSettings.enableParameters) {

		ImGui::Separator();
		ImGui::Text("Sensor Distance");
		ImGui::SliderFloat("p1", &appState->parameterSettings.p1, 0.0f, 30.0f);
		ImGui::SliderFloat("p2", &appState->parameterSettings.p2, 0.0f, 30.0f);
		ImGui::SliderFloat("p3", &appState->parameterSettings.p3, 0.0f, 30.0f);

		ImGui::Separator();
		ImGui::Text("Sensor Angle");
		ImGui::SliderFloat("p4", &appState->parameterSettings.p4, 0.0f, 30.0f);
		ImGui::SliderFloat("p5", &appState->parameterSettings.p5, 0.0f, 30.0f);
		ImGui::SliderFloat("p6", &appState->parameterSettings.p6, 0.0f, 30.0f);

		ImGui::Separator();
		ImGui::Text("Rotation Angle");
		ImGui::SliderFloat("p7", &appState->parameterSettings.p7, 0.0f, 30.0f);
		ImGui::SliderFloat("p8", &appState->parameterSettings.p8, 0.0f, 30.0f);
		ImGui::SliderFloat("p9", &appState->parameterSettings.p9, 0.0f, 30.0f);

		ImGui::Separator();
		ImGui::Text("Move Distance");
		ImGui::SliderFloat("p10", &appState->parameterSettings.p10, 0.0f, 30.0f);
		ImGui::SliderFloat("p11", &appState->parameterSettings.p11, 0.0f, 30.0f);
		ImGui::SliderFloat("p12", &appState->parameterSettings.p12, 0.0f, 30.0f);
	}

	ImGui::End();
	
	if (appState->lockAngles) {
		appState->slimeSettings.angle = appState->slimeSettings.rotationAngle;
	}
	
	if (appState->lockSlimeColor) {
		appState->slimeSettings.slimeColor1 = appState->slimeSettings.slimeColor0;
		appState->slimeSettings.slimeColor2 = appState->slimeSettings.slimeColor0;
	}
	
	if (appState->lockParticleColor) {
		appState->slimeSettings.particleColor1 = appState->slimeSettings.particleColor0;
		appState->slimeSettings.particleColor2 = appState->slimeSettings.particleColor0;
	}
}