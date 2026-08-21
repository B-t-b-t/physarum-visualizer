#include "audio_window.h"

#include <cmath>
#include <map>
#include <vector>

#include "implot.h"

#include "../ui_helpers.h"
#include "../../audio/audio_device_manager.h"

AudioWindow::AudioWindow() {
    heatmapData_.reserve(512 * 32);
	heatmapData_.resize(512 * 32, 0.0);

	heatMapChange_.reserve(512 * 32);
	heatMapChange_.resize(512 * 32, 0.0);

	frequencyXAxis_ = new std::vector<double>();
	for(int i = 0; i < 2048; i++) {
		frequencyXAxis_->push_back(i * ((24000 - 11.7188) / 2048));
	}
}

void AudioWindow::render(ApplicationState* appState) {
    if(!visible) { return; }
    
    ImGui::Begin("Audio", &visible);

	ImGui::Checkbox("Enable Audio Processing", (bool*)&appState->slimeSettings.reactToAudio);
	ImGui::SameLine(); HelpMarker("When enabled, the slime movement will react to audio input. Make sure to select an audio input device in the Audio menu.");
	
	std::map<SDL_AudioDeviceID, AudioDeviceInfo>* availableDevices = appState->availableAudioHardware;

	if (ImGui::BeginListBox("Audio Hardware")) {
		for (const auto& deviceEntry : *availableDevices) {
			const bool is_selected = deviceEntry.second.logicalID != 0;
			if (ImGui::Selectable(deviceEntry.second.name, is_selected)) {
				appState->usedAudioHardwareIndex = deviceEntry.first;
				notify(Event::AUDIO_HARDWARE_CHANGE);
			}
			// Set the initial focus when opening the combo (scrolling + keyboard navigation focus)
			if (is_selected) {
				ImGui::SetItemDefaultFocus();
			}
		}

		ImGui::EndListBox();
	}

    if (ImGui::CollapsingHeader("Audio Graph")) {
		if (ImPlot::BeginPlot("Audio")) {
			ImPlotSpec spec;
			spec.LineColor = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
			spec.Flags = ImPlotItemFlags_NoLegend;
			ImPlotAxisFlags axes_flags = ImPlotAxisFlags_AutoFit | ImPlotAxisFlags_NoMenus | ImPlotAxisFlags_NoSideSwitch | ImPlotAxisFlags_NoHighlight;

			ImPlot::SetupAxes("t","y", axes_flags, axes_flags);
			ImPlot::SetupAxisLimits(ImAxis_Y1, -1.1, 1.1, ImGuiCond_Always);
			ImPlot::SetupAxisFormat(ImAxis_X1, TimeFormatter, (void*)"s");
			ImPlot::SetupFinish();
			ImPlot::PlotLineG("Audio##2", MyDataGetter, appState->audioBuffer->data(), appState->audioBuffer->size(), spec);
			ImPlot::EndPlot();
		}
	}
	if (ImGui::CollapsingHeader("Audio Spectrum")) {

		const double start_subBass = 20;
		const double start_bass = 60;
		const double start_lowMidRange = 250;
		const double start_midRange = 500;
		const double start_upperMidRange = 2000;
		const double start_presence = 4000;
		const double start_brilliance = 6000;
		const double end_brilliance = 20000;

		static double freqBands[] = {start_subBass, start_bass, start_lowMidRange, start_midRange, start_upperMidRange, start_presence, start_brilliance, end_brilliance};

		if (ImPlot::BeginPlot("Audio Spectrum##2")) {

			ImPlotAxisFlags axes_flags = ImPlotAxisFlags_AutoFit | ImPlotAxisFlags_NoMenus | ImPlotAxisFlags_NoSideSwitch | ImPlotAxisFlags_NoHighlight;
			ImPlot::SetupAxes("Frequency [Hz]", "Magnitude", axes_flags, axes_flags);
			ImPlot::SetupAxisLimits(ImAxis_Y1, 0, 100, ImGuiCond_Always);

			ImPlot::SetupAxisScale(ImAxis_X1, ImPlotScale_Log10);
			ImPlot::SetupAxisFormat(ImAxis_X1, FrequencyFormatter, (void*)"Hz");
			ImPlot::SetupAxisLimits(ImAxis_X1, 16, 24000, ImGuiCond_Always);
			
			ImPlot::PushStyleColor(ImPlotCol_AxisText, ImVec4(1.0f, 0.78f, 0.39f, 0.9f));
			ImPlot::SetupAxis(ImAxis_X2, "Frequency Bands [Hz]", ImPlotAxisFlags_Opposite | ImPlotAxisFlags_NoGridLines | ImPlotAxisFlags_NoLabel);
			ImPlot::SetupAxisScale(ImAxis_X2, ImPlotScale_Log10);
			ImPlot::SetupAxisFormat(ImAxis_X2, FrequencyFormatter, (void*)"Hz");
			ImPlot::SetupAxisLimits(ImAxis_X2, 16, 24000, ImGuiCond_Always);

			ImPlot::SetupAxisTicks(ImAxis_X2, &start_subBass, 1, nullptr, false);
			ImPlot::SetupAxisTicks(ImAxis_X2, &start_bass, 1, nullptr, false);
			ImPlot::SetupAxisTicks(ImAxis_X2, &start_lowMidRange, 1, nullptr, false);
			ImPlot::SetupAxisTicks(ImAxis_X2, &start_midRange, 1, nullptr, false);
			ImPlot::SetupAxisTicks(ImAxis_X2, &start_upperMidRange, 1, nullptr, false);
			ImPlot::SetupAxisTicks(ImAxis_X2, &start_presence, 1, nullptr, false);
			ImPlot::SetupAxisTicks(ImAxis_X2, &start_brilliance, 1, nullptr, false);
			ImPlot::SetupAxisTicks(ImAxis_X2, &end_brilliance, 1, nullptr, false);
			ImPlot::PopStyleColor();
			
			ImPlot::SetupFinish();

			ImPlotSpec spec;
			spec.LineColor = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
			spec.Flags = ImPlotItemFlags_NoLegend;
			ImPlot::PlotLine("Spectrum", frequencyXAxis_->data(), appState->spectrum->data(), 2048, spec);

			ImPlot::PushStyleColor(ImPlotCol_InlayText, ImVec4(1.0f, 0.78f, 0.39f, 0.9f));
			ImPlot::PlotText("Sub Bass", (start_bass + start_subBass) / 2 - 5, 70);
			ImPlot::PlotText("Bass", (start_lowMidRange + start_bass) / 2 - 30, 70);
			ImPlot::PlotText("     Low\nMid Range", (start_midRange + start_lowMidRange) / 2 - 20, 70);
			ImPlot::PlotText("Mid Range", (start_upperMidRange + start_midRange) / 2 - 250, 70);
			ImPlot::PlotText("   Upper\nMid Range", (start_presence + start_upperMidRange) / 2 - 100, 70);
			ImPlot::PlotText("Presence", (start_brilliance + start_presence) / 2 - 50, 70);
			ImPlot::PlotText("Brilliance", (end_brilliance + start_brilliance) / 2 - 2000, 70);

			ImPlot::PopStyleColor();
			
			ImPlotSpec lineSpec;
			lineSpec.LineColor = ImVec4(1.0f, 0.78f, 0.39f, 0.9f);
			lineSpec.Flags = ImPlotItemFlags_NoLegend;
			ImPlot::PlotInfLines("Frequency Bands", freqBands, 8, lineSpec);
			ImPlot::EndPlot();
		}
	}

	// Heatmap
	if (ImGui::CollapsingHeader("Spectrum Heatmap")) {
		if(*(appState->hasNewSpectrumData))  {
			for (size_t i = 0; i < 512 * 32; i += 32) {
				double spectrumVal = (*(appState->spectrum))[i / 32];
				spectrumVal = std::isnan(spectrumVal) ? 0.0 : spectrumVal;
				heatmapData_[i + (static_cast<size_t>(heatMapIndex_))] = spectrumVal;

				double diffVal = (*(appState->spectrumDiff))[i / 32];
				diffVal = std::isnan(diffVal) ? 0.0 : diffVal;
				heatMapChange_[i + (static_cast<size_t>(heatMapIndex_))] = diffVal > 0 ? diffVal : 0.0;
			}

			heatMapIndex_ = (heatMapIndex_ + 1) % 32;
		}

		static float scale_min       = 0.0f;
		static float scale_max       = 100;
		static const char* xlabels[] = {"0", "0.68", "1.37", "2.05", "2.73"};
		static const char* ylabels[] = {"0", "3000", "6000", "9000", "12000", "15000", "18000", "21000", "24000"};

		static ImPlotColormap map = ImPlotColormap_Hot;

		ImGui::SetNextItemWidth(225);
		ImGui::DragFloatRange2("Min / Max",&scale_min, &scale_max, 0.01f, -20, 20);
		
		static ImPlotAxisFlags axes_flags = ImPlotAxisFlags_Lock | ImPlotAxisFlags_NoGridLines | ImPlotAxisFlags_NoMenus | ImPlotAxisFlags_NoSideSwitch | ImPlotAxisFlags_NoHighlight;
		
		ImPlot::PushColormap(map);

		if (ImPlot::BeginPlot("##Spectrum History",ImVec2(512,768),ImPlotFlags_NoLegend|ImPlotFlags_NoMouseText)) {
			ImPlot::SetupAxes("Time [s]", "Frequency [Hz]", axes_flags, axes_flags);
			ImPlot::SetupAxisTicks(ImAxis_X1, 0.0, 1.0, 5, xlabels);
			ImPlot::SetupAxisTicks(ImAxis_Y1, 0.0, 1.0, 9, ylabels);
			ImPlot::SetupFinish();
			ImPlotSpec spec;
			ImPlot::PlotHeatmap("heat", heatmapData_.data(), 512, 32, scale_min, scale_max, nullptr, ImPlotPoint(0,1), ImPlotPoint(1,0), spec);
			ImPlot::EndPlot();
		}

		ImGui::SameLine();

		if (ImPlot::BeginPlot("##Spectrum Change History",ImVec2(512,768),ImPlotFlags_NoLegend|ImPlotFlags_NoMouseText)) {
			ImPlot::SetupAxes("Time [s]", "Frequency [Hz]", axes_flags, axes_flags);
			ImPlot::SetupAxisTicks(ImAxis_X1, 0.0, 1.0, 5, xlabels);
			ImPlot::SetupAxisTicks(ImAxis_Y1, 0.0, 1.0, 9, ylabels);
			ImPlot::SetupFinish();
			ImPlotSpec spec;
			ImPlot::PlotHeatmap("heat", heatMapChange_.data(), 512, 32, scale_min, scale_max, nullptr, ImPlotPoint(0,1), ImPlotPoint(1,0), spec);
			ImPlot::EndPlot();
		}
		ImPlot::PopColormap();	//to avoid interfering with the colour of other plots when the draw order changes (Collapsing Headers, ...)
		
	}

	// ImGui::SliderInt("Scan Time [ms]", &audioTimer, 1, 1000);
	// if (ImGui::IsItemDeactivatedAfterEdit()) {
	// 	std::cout << "Hi" << std::endl;
	// 	SDL_RemoveTimer(timerID);
	// 	timerID = SDL_AddTimer(audioTimer, getAudioCallback, &data);
	// }

	// float minFreqency = 1 / (BUFFER_SIZE / (double)AudioRate);

	ImGui::SeparatorText("Beat Detection");

	ImGui::Text("VelocityBassReaction: %.2f", appState->slimeSettings.velocityBassReaction);

	HelpMarker("Frequency Range: 20-60 Hz");
	ImGui::SameLine();
	ImGui::Text("Sub Bass Detected: %d", appState->subBassDetected);
	ImGui::SameLine();
	ImGui::Text("Sub Bass Value: %.2f", appState->subBassValue);

	HelpMarker("Frequency Range: 60-250 Hz");
	ImGui::SameLine();
	ImGui::Text("Bass Detected: %d", appState->bassDetected);
	ImGui::SameLine();
	ImGui::Text("Bass Value: %.2f", appState->bassValue);

	HelpMarker("Frequency Range: 250-500 Hz");
	ImGui::SameLine();
	ImGui::Text("Low Mid Range Detected: %d", appState->lowMidRangeDetected);
	ImGui::SameLine();
	ImGui::Text("Low Mid Range Value: %.2f", appState->lowMidRangeValue);

	HelpMarker("Frequency Range: 500-2000 Hz");
	ImGui::SameLine();
	ImGui::Text("Mid Range Detected: %d", appState->midRangeDetected);
	ImGui::SameLine();
	ImGui::Text("Mid Range Value: %.2f", appState->midRangeValue);

	HelpMarker("Frequency Range: 2000-4000 Hz");
	ImGui::SameLine();
	ImGui::Text("Upper Mid Range Detected: %d", appState->upperMidRangeDetected);
	ImGui::SameLine();
	ImGui::Text("Upper Mid Range Value: %.2f", appState->upperMidRangeValue);

	HelpMarker("Frequency Range: 4000-6000 Hz");
	ImGui::SameLine();
	ImGui::Text("Presence Detected: %d", appState->presenceDetected);
	ImGui::SameLine();
	ImGui::Text("Presence Value: %.2f", appState->presenceValue);

	HelpMarker("Frequency Range: 6000-20000 Hz");
	ImGui::SameLine();
	ImGui::Text("Brilliance Detected: %d", appState->brillianceDetected);
	ImGui::SameLine();
	ImGui::Text("Brilliance Value: %.2f", appState->brillianceValue);

	ImGui::SliderFloat("Beat Divide", &appState->beatDivide, 0, 100.0f);
	//ImGui::Checkbox("Beat Normalization", &appState->normalizeBeat);
	// ImGui::SeparatorText("Fringe Detection (non functional)");
	// ImGui::SliderInt("Fringe Beginn", &state_.fringeBeginn, 0, state_.fringeEnd);
	// ImGui::SliderInt("Fringe End", &state_.fringeEnd, state_.fringeBeginn, bufferSize / 2);
	// ImGui::SliderInt("Fringe Divide", &state_.fringeDivide, 1, 1000);

	ImGui::End();

	*(appState->hasNewSpectrumData) = false;
}