#ifndef UI_HELPERS_H
#define UI_HELPERS_H

#include "implot.h"

void HelpMarker(const char* desc);

ImPlotPoint MyDataGetter(int idx, void* data) ;

int TimeFormatter(double value, char* buff, int size, void* data);

int FrequencyFormatter(double value, char* buff, int size, void* data);

#endif // UI_HELPERS_H