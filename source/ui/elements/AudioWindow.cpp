#include "AudioWindow.h"
#include "../UIHelpers.h"
#include "implot.h"

AudioWindow::AudioWindow() {
    heatmapData_.reserve(512 * 32);
	heatmapData_.resize(512 * 32, 0.0);

	heatMapChange_.reserve(512 * 32);
	heatMapChange_.resize(512 * 32, 0.0);
}

void AudioWindow::render(UIState& state) {
    if(!visible) { return; }
    
    ImGui::Begin("Audio", &visible);

	ImGui::Checkbox("Enable Audio Processing", (bool*)&state.slimeSettings.reactToAudio);
	ImGui::SameLine(); HelpMarker("When enabled, the slime movement will react to audio input. Make sure to select an audio input device in the Audio menu.");
	
    if (ImGui::CollapsingHeader("Audio Graph")) {
		if (ImPlot::BeginPlot("Audio")) {
			ImPlot::SetupAxes("t","y", ImPlotAxisFlags_None, ImPlotAxisFlags_AutoFit);
			ImPlot::SetupAxisFormat(ImAxis_X1, TimeFormatter, (void*)"s");
			ImPlot::SetupFinish();
			ImPlot::PlotLineG("Audio##2", MyDataGetter, audioBuffer_->data(), bufferSize_);
			//ImPlot::PlotLine("Audio", audioBuffer.data(), bufferSize);
			ImPlot::SetNextMarkerStyle(ImPlotMarker_Circle);
			ImPlot::EndPlot();
		}
	}
	if (ImGui::CollapsingHeader("Audio Spectrum")) {

		if (ImPlot::BeginPlot("Audio Spectrum##2")) {
			ImPlot::SetupAxes("Frequency (Hz)", "Magnitude", ImPlotAxisFlags_None, ImPlotAxisFlags_AutoFit);
			//static ImPlotDragToolFlags flags = ImPlotDragToolFlags_None;
			//ImPlot::SetupAxisLimits(ImAxis_Y1, 0, 700, ImGuiCond_Always);
			ImPlot::SetupAxisFormat(ImAxis_X1, FrequencyFormatter, (void*)"Hz");
			//ImPlot::SetupAxisScale(ImAxis_Y1, ImPlotScale_Log10);  // Log scale for magnitude
			//double beatBeginnFreq = 11.0f;//state_.beatBeginn * (1 / (bufferSize / (double)audioRate));
			ImPlot::SetupFinish();
			ImPlot::PlotLine("Spectrum", spectrum_->data(), 2048);
			//ImPlot::PlotInfLines("BeatRange", beatRange, 2);
			//ImPlot::PlotInfLines("FringeRange", fringeRange, 2);
			//ImPlot::PlotInfLines("BeatLimits", beatLimits, 2, ImPlotInfLinesFlags_Horizontal);
			//ImPlot::PlotBars("Spectrum", spectrum.data(), 100, 1.0, 0.5);

			ImPlot::EndPlot();
		}
	}

	// Heatmap
	if(hasNewSpectrumData_)  {
		for (size_t i = 0; i < 512 * 32; i = i + 32) {
			heatmapData_[i + (static_cast<size_t>(heatMapIndex_))] = (*spectrum_)[i / 32];
		}

		for (size_t i = 0; i < 512 * 32; i = i + 32) {
			double diffVal = (*spectrumDiff_)[i / 32];
			heatMapChange_[i + (static_cast<size_t>(heatMapIndex_))] = diffVal > 0 ? diffVal : 0.0;
		}

		heatMapIndex_ = (heatMapIndex_ + 1) % 32;
	}

    static float scale_min       = 0.0f;
    static float scale_max       = 100;
    //static const char* xlabels[] = {"C1","C2","C3","C4","C5","C6","C7"};
    //static const char* ylabels[] = {"R1","R2","R3","R4","R5","R6","R7"};

    static ImPlotColormap map = ImPlotColormap_Hot;

    ImGui::SameLine();
    ImGui::LabelText("##Colormap Index", "%s", "Change Colormap");
    ImGui::SetNextItemWidth(225);
    ImGui::DragFloatRange2("Min / Max",&scale_min, &scale_max, 0.01f, -20, 20);

    static ImPlotAxisFlags axes_flags = ImPlotAxisFlags_Lock | ImPlotAxisFlags_NoGridLines | ImPlotAxisFlags_NoTickMarks;

	ImPlot::PushColormap(map);

	if (ImPlot::BeginPlot("##Heatmap1",ImVec2(512,768),ImPlotFlags_NoLegend|ImPlotFlags_NoMouseText)) {
		ImPlot::SetupAxes("Time", "Frequency (Hz)", axes_flags, axes_flags);
        //ImPlot::SetupAxisTicks(ImAxis_X1,0 + 1.0/14.0, 1 - 1.0/14.0, 7, xlabels);
		ImPlot::SetupFinish();
        ImPlot::PlotHeatmap("heat", heatmapData_.data(), 512, 32, scale_min, scale_max, nullptr, ImPlotPoint(0,1), ImPlotPoint(1,0), 0);
        ImPlot::EndPlot();
    }

	ImGui::SameLine();

	if (ImPlot::BeginPlot("##Heatmap2",ImVec2(512,768),ImPlotFlags_NoLegend|ImPlotFlags_NoMouseText)) {
		ImPlot::SetupAxes("Time", "Frequency (Hz)", axes_flags, axes_flags);
        //ImPlot::SetupAxisTicks(ImAxis_X1,0 + 1.0/14.0, 1 - 1.0/14.0, 7, xlabels);
		ImPlot::SetupFinish();
        ImPlot::PlotHeatmap("heat", heatMapChange_.data(), 512, 32, scale_min, scale_max, nullptr, ImPlotPoint(0,1), ImPlotPoint(1,0), 0);
        ImPlot::EndPlot();
    }
	ImPlot::PopColormap();	//to avoid interfering with the colour of other plots when the draw order changes (Collapsing Headers, ...)

	// ImGui::SliderInt("Scan Time [ms]", &audioTimer, 1, 1000);
	// if (ImGui::IsItemDeactivatedAfterEdit()) {
	// 	std::cout << "Hi" << std::endl;
	// 	SDL_RemoveTimer(timerID);
	// 	timerID = SDL_AddTimer(audioTimer, getAudioCallback, &data);
	// }

	// float minFreqency = 1 / (BUFFER_SIZE / (double)AudioRate);

	ImGui::SeparatorText("Beat Detection");

	ImGui::Text("VelocityBassReaction: %.2f", state.slimeSettings.velocityBassReaction);

	HelpMarker("Frequency Range: 20-60 Hz");
	ImGui::SameLine();
	ImGui::Text("Sub Bass Detected: %d", state.subBassDetected);
	ImGui::SameLine();
	ImGui::Text("Sub Bass Value: %.2f", state.subBassValue);

	HelpMarker("Frequency Range: 60-250 Hz");
	ImGui::SameLine();
	ImGui::Text("Bass Detected: %d", state.bassDetected);
	ImGui::SameLine();
	ImGui::Text("Bass Value: %.2f", state.bassValue);

	HelpMarker("Frequency Range: 250-500 Hz");
	ImGui::SameLine();
	ImGui::Text("Low Mid Range Detected: %d", state.lowMidRangeDetected);
	ImGui::SameLine();
	ImGui::Text("Low Mid Range Value: %.2f", state.lowMidRangeValue);

	HelpMarker("Frequency Range: 500-2000 Hz");
	ImGui::SameLine();
	ImGui::Text("Mid Range Detected: %d", state.midRangeDetected);
	ImGui::SameLine();
	ImGui::Text("Mid Range Value: %.2f", state.midRangeValue);

	HelpMarker("Frequency Range: 2000-4000 Hz");
	ImGui::SameLine();
	ImGui::Text("Upper Mid Range Detected: %d", state.upperMidRangeDetected);
	ImGui::SameLine();
	ImGui::Text("Upper Mid Range Value: %.2f", state.upperMidRangeValue);

	HelpMarker("Frequency Range: 4000-6000 Hz");
	ImGui::SameLine();
	ImGui::Text("Presence Detected: %d", state.presenceDetected);
	ImGui::SameLine();
	ImGui::Text("Presence Value: %.2f", state.presenceValue);

	HelpMarker("Frequency Range: 6000-20000 Hz");
	ImGui::SameLine();
	ImGui::Text("Brilliance Detected: %d", state.brillianceDetected);
	ImGui::SameLine();
	ImGui::Text("Brilliance Value: %.2f", state.brillianceValue);

	ImGui::SliderFloat("Beat Divide", &state.beatDivide, 0, 100.0f);
	ImGui::Checkbox("Beat Normalization", &state.normalizeBeat);
	// ImGui::SeparatorText("Fringe Detection (non functional)");
	// ImGui::SliderInt("Fringe Beginn", &state_.fringeBeginn, 0, state_.fringeEnd);
	// ImGui::SliderInt("Fringe End", &state_.fringeEnd, state_.fringeBeginn, bufferSize / 2);
	// ImGui::SliderInt("Fringe Divide", &state_.fringeDivide, 1, 1000);
	
	if (ImGui::BeginListBox("Audio Hardware")) {

		for (unsigned int n = 0; n < availableHardwareDevices_.size(); n++)
		{
			const bool is_selected = (selectedHardwareDevice_ == n);
			if (ImGui::Selectable(availableHardwareDevices_[n].c_str(), is_selected)) {
				selectedHardwareDevice_ = n;
				state.selectAudioHardware = true;
			}
			// Set the initial focus when opening the combo (scrolling + keyboard navigation focus)
			if (is_selected) {
				ImGui::SetItemDefaultFocus();
			}
		}

		ImGui::EndListBox();
	}

	ImGui::End();
}

void AudioWindow::update(std::vector<double>& audioBuffer, std::vector<double>& spectrum, std::vector<double>& spectrumDiff, int bufferSize, bool hasNewSpectrumData) {
    audioBuffer_ = &audioBuffer;
    spectrum_ = &spectrum;
    spectrumDiff_ = &spectrumDiff;
    bufferSize_ = bufferSize;
    hasNewSpectrumData_ = hasNewSpectrumData;
}