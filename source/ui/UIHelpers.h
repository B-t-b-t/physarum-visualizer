#pragma once
#include "implot.h"

void HelpMarker(const char* desc);

ImPlotPoint MyDataGetter(int idx, void* data) ;

int TimeFormatter(double value, char* buff, int size, void* data);

int FrequencyFormatter(double value, char* buff, int size, void* data);