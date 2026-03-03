#include "UIHelpers.h"
#include "imgui.h"
#include <math.h>
#include <iostream>

// Helper to display a little (?) mark which shows a tooltip when hovered.
// In your own code you may want to display an actual icon if you are using a merged icon fonts (see docs/FONTS.md)
void HelpMarker(const char* desc)
{
    ImGui::TextDisabled("(?)");
    if (ImGui::BeginItemTooltip())
    {
        ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);
        ImGui::TextUnformatted(desc);
        ImGui::PopTextWrapPos();
        ImGui::EndTooltip();
    }
}

ImPlotPoint MyDataGetter(int idx, void* data) {
	double* my_data = (double*)data;
	ImPlotPoint p;
	p.x = idx;
	p.y = (double)my_data[idx];
	return p;
}

int TimeFormatter(double value, char* buff, int size, void* data) {
    const char* unit = (const char*)data;
    static double v[]      = {1000000000,1000000,1000,1,0.001,0.000001,0.000000001};
    static const char* p[] = {"G","M","k","","m","u","n"};
	value = value / 48000;
	if (fabs(value) < std::numeric_limits<double>::epsilon()) {
		return snprintf(buff,static_cast<size_t>(size),"0 %s", unit);
	}
    for (int i = 0; i < 7; ++i) {
        if (fabs(value) >= v[i]) {
            return snprintf(buff,static_cast<size_t>(size),"%g %s%s",value/v[i],p[i],unit);
        }
    }
    return snprintf(buff,static_cast<size_t>(size),"%g %s%s",value/v[6],p[6],unit);
}

int FrequencyFormatter(double value, char* buff, int size, void* data) {
    const char* unit = (const char*)data;
    static double v[]      = {1000000000,1000000,1000,1,0.001,0.000001,0.000000001};
    static const char* p[] = {"G","M","k","","m","u","n"};
	value = value * (1 / (4096 / (double)48000));
    if (fabs(value) < std::numeric_limits<double>::epsilon()) {
        return snprintf(buff,static_cast<size_t>(size),"0 %s", unit);
    }
    for (int i = 0; i < 7; ++i) {
        if (fabs(value) >= v[i]) {
            return snprintf(buff,static_cast<size_t>(size),"%g %s%s",value/v[i],p[i],unit);
        }
    }
    return snprintf(buff,static_cast<size_t>(size),"%g %s%s",value/v[6],p[6],unit);
}