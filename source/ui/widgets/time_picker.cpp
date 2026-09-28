#include "time_picker.h"

#include <array>

#include "imgui.h"

namespace ImGui {

    bool timePicker(const char* ID, std::chrono::system_clock::time_point* time) {
        using namespace std::chrono;
        
        //split into date and daytime components
        year_month_day date = year_month_day{floor<days>(*time)};
        hh_mm_ss<seconds> dayTime = hh_mm_ss{floor<seconds>(*time) - floor<days>(*time)};

        timePicker(ID, &dayTime);

        //reconstruct the modified timepoint from date and daytime
        *time = sys_days{date} + dayTime.to_duration();

        return true;
    }

    bool timePicker(const char* ID, std::chrono::hh_mm_ss<std::chrono::seconds>* time) {
        static std::array<const char*, 60> numArray = {"00", "01", "02", "03", "04", "05", "06", "07", "08", "09",
                                              "10", "11", "12", "13", "14", "15", "16", "17", "18", "19",
                                              "20", "21", "22", "23", "24", "25", "26", "27", "28", "29",
                                              "30", "31", "32", "33", "34", "35", "36", "37", "38", "39",
                                              "40", "41", "42", "43", "44", "45", "46", "47", "48", "49",
                                              "50", "51", "52", "53", "54", "55", "56", "57", "58", "59"};

        size_t selectedHour = (size_t)time->hours().count();
        size_t selectedMinute = (size_t)time->minutes().count();
        size_t selectedSecond = (size_t)time->seconds().count();

        char hourStr[12];   //can hold length of e.g. "00##second" and null terminator
        char minuteStr[12];
        char secondStr[12];
        sprintf(hourStr, "%s##hour", numArray[selectedHour]);
        sprintf(minuteStr, "%s##minute", numArray[selectedMinute]);
        sprintf(secondStr, "%s##second", numArray[selectedSecond]);

        ImGui::PushID(ID);

        ImGui::PushStyleColor(ImGuiCol_Button, ImGuiCol_WindowBg);
        //reduce space between buttons for a better look
        ImGui::PushStyleVarX(ImGuiStyleVar_ItemSpacing, (float)(int)(ImGui::GetStyle().ItemSpacing.x * 0.1));

        static ImVec2 pos_min{};
        static ImVec2 pos_max{};

        if(ImGui::Button(hourStr)) {
            pos_min = ImGui::GetItemRectMin();
            pos_max = ImGui::GetItemRectMax();
            ImGui::OpenPopup("hour_popup"); 
        }
        ImGui::SameLine();
        ImGui::Text(":");
        ImGui::SameLine();
        if(ImGui::Button(minuteStr)) {
            pos_min = ImGui::GetItemRectMin();
            pos_max = ImGui::GetItemRectMax();
            ImGui::OpenPopup("minute_popup");
        }
        ImGui::SameLine();
        ImGui::Text(":");
        ImGui::SameLine();
        if(ImGui::Button(secondStr)) {
            pos_min = ImGui::GetItemRectMin();
            pos_max = ImGui::GetItemRectMax();
            ImGui::OpenPopup("second_popup");
        }

        ImGui::PopStyleVar();   //restore original item spacing
        ImGui::PopStyleColor(); //restore original button color

        ImGuiTableFlags table_flags = ImGuiTableFlags_ScrollY;
        ImVec2 tableHeight = ImVec2(0.0f, ImGui::GetTextLineHeight() * 5);

        //shift Popup so, that the number on the button and the on the appearing popup align
        //TODO: convert magic numbers to calculations
        ImVec2 pivotHour{0.33, 0.5};
        ImVec2 pivotMinute{0.33, 0.5};
        ImVec2 pivotSecond{0.33, 0.5};

        //edge cases for start and end of the scrollable range
        switch(selectedHour) {
            case 0:  pivotHour = ImVec2(0.33, 0.18); break;
            case 1:  pivotHour = ImVec2(0.33, 0.39); break;
            case 22: pivotHour = ImVec2(0.33, 0.60); break;
            case 23: pivotHour = ImVec2(0.33, 0.81); break;
        }

        switch(selectedMinute) {
            case 0:  pivotMinute = ImVec2(0.33, 0.18); break;
            case 1:  pivotMinute = ImVec2(0.33, 0.39); break;
            case 58: pivotMinute = ImVec2(0.33, 0.60); break;
            case 59: pivotMinute = ImVec2(0.33, 0.81); break;
        }

        switch(selectedSecond) {
            case 0:  pivotSecond = ImVec2(0.33, 0.18); break;
            case 1:  pivotSecond = ImVec2(0.33, 0.39); break;
            case 58: pivotSecond = ImVec2(0.33, 0.60); break;
            case 59: pivotSecond = ImVec2(0.33, 0.81); break;
        }

        ImGui::SetNextWindowPos(ImVec2((pos_max.x + pos_min.x) / 2.0f, (pos_max.y + pos_min.y) / 2.0f), 0, pivotHour);

        if (ImGui::BeginPopup("hour_popup")) {
            if (ImGui::BeginTable("hourSelection", 1, table_flags, tableHeight)) {
                for (size_t hour = 0; hour < 24; ++hour) {
                    ImGui::TableNextRow();
                    ImGui::TableNextColumn();
                    bool isSelected = (selectedHour == hour);
                    if(ImGui::Selectable(numArray[hour], isSelected)) { 
                        selectedHour = hour; 
                        ImGui::CloseCurrentPopup();
                    }

                    if(ImGui::IsWindowAppearing() && isSelected) { ImGui::SetScrollHereY(); }
                }
                ImGui::EndTable();
            }
            ImGui::EndPopup();
        }

        ImGui::SetNextWindowPos(ImVec2((pos_max.x + pos_min.x) / 2.0f, (pos_max.y + pos_min.y) / 2.0f), 0, pivotMinute);

        if (ImGui::BeginPopup("minute_popup")) {
            if (ImGui::BeginTable("minuteSelection", 1, table_flags, tableHeight)) {
                for (size_t minute = 0; minute < 60; ++minute) {
                    ImGui::TableNextRow();
                    ImGui::TableNextColumn();
                    bool isSelected = (selectedMinute == minute);
                    if(ImGui::Selectable(numArray[minute], isSelected)) { 
                        selectedMinute = minute; 
                        ImGui::CloseCurrentPopup();
                    }

                    if(ImGui::IsWindowAppearing() && isSelected) { ImGui::SetScrollHereY(); }
                }
                ImGui::EndTable();
            }
            ImGui::EndPopup();
        }
        
        ImGui::SetNextWindowPos(ImVec2((pos_max.x + pos_min.x) / 2.0f, (pos_max.y + pos_min.y) / 2.0f), 0, pivotSecond);

        if (ImGui::BeginPopup("second_popup")) {
            if (ImGui::BeginTable("secondSelection", 1, table_flags, tableHeight)) {
                for (size_t second = 0; second < 60; ++second) {
                    ImGui::TableNextRow();
                    ImGui::TableNextColumn();
                    bool isSelected = (selectedSecond == second);
                    if(ImGui::Selectable(numArray[second], isSelected)) { 
                        selectedSecond = second; 
                        ImGui::CloseCurrentPopup();
                    }

                    if(ImGui::IsWindowAppearing() && isSelected) { ImGui::SetScrollHereY(); }
                }
                ImGui::EndTable();
            }
            ImGui::EndPopup();
        }

        ImGui::PopID(); //ID of this time picker widget

        *time = std::chrono::hh_mm_ss<std::chrono::seconds>{std::chrono::hours(selectedHour) + std::chrono::minutes(selectedMinute) + std::chrono::seconds(selectedSecond)};

        return true;
    }

    bool datePicker(const char* ID, std::chrono::system_clock::time_point* time) {
        using namespace std::chrono;
        
        //split into date and daytime components
        year_month_day date = year_month_day{floor<days>(*time)};
        hh_mm_ss<seconds> dayTime = hh_mm_ss{floor<seconds>(*time) - floor<days>(*time)};

        const size_t dayArrayLength = 31;
        const size_t monthArrayLength = 12;
        const size_t daysPerMonthLength = 12;
        const size_t yearArrayLength = 60;

        static std::array<const char*, dayArrayLength> dayArray = {"01", "02", "03", "04", "05", "06", "07", "08", "09", "10",
            "11", "12", "13", "14", "15", "16", "17", "18", "19", "20",
            "21", "22", "23", "24", "25", "26", "27", "28", "29", "30", "31"};
        
        static std::array<const char*, monthArrayLength> monthArray = {"Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};
        static std::array<const size_t, daysPerMonthLength> daysPerMonth = {   31,    28,    31,    30,    31,    30,    31,    31,    30,    31,    30,    31};


            static std::array<const char*, yearArrayLength> yearArray = {"2026", "2027", "2028", "2029", "2030", "2031", "2032", "2033", "2034", "2035",
                                                             "2036", "2037", "2038", "2039", "2040", "2041", "2042", "2043", "2044", "2045",
                                                             "2046", "2047", "2048", "2049", "2050", "2051", "2052", "2053", "2054", "2055",
                                                             "2056", "2057", "2058", "2059", "2060", "2061", "2062", "2063", "2064", "2065",
                                                             "2066", "2067", "2068", "2069", "2070", "2071", "2072", "2073", "2074", "2075",
                                                             "2076", "2077", "2078", "2079", "2080", "2081", "2082", "2083", "2084", "2085"};
            
        size_t selectedDayIndex = (size_t)static_cast<unsigned int>(date.day()) - 1;
        size_t selectedMonthIndex = (size_t)static_cast<unsigned int>(date.month()) - 1;
        size_t selectedYearIndex = (size_t)static_cast<int>(date.year()) - 2026;

        char dayStr[12];    //can hold length of e.g. "00##second" and null terminator
        char monthStr[12];
        char yearStr[12];
        sprintf(dayStr, "%s##day", dayArray[selectedDayIndex]);
        sprintf(monthStr, "%s##month", monthArray[selectedMonthIndex]);
        sprintf(yearStr, "%s##year", yearArray[selectedYearIndex]);

        ImGui::PushID(ID);

        ImGui::PushStyleColor(ImGuiCol_Button, ImGuiCol_WindowBg);
        //reduce space between buttons for a better look
        ImGui::PushStyleVarX(ImGuiStyleVar_ItemSpacing, (float)(int)(ImGui::GetStyle().ItemSpacing.x * 0.1));

        static ImVec2 pos_min{};
        static ImVec2 pos_max{};

        if(ImGui::Button(dayStr)) {
            pos_min = ImGui::GetItemRectMin();
            pos_max = ImGui::GetItemRectMax();
            ImGui::OpenPopup("day_popup");
        }
        ImGui::SameLine();
        ImGui::Text(".");
        ImGui::SameLine();
        if(ImGui::Button(monthStr)) {
            pos_min = ImGui::GetItemRectMin();
            pos_max = ImGui::GetItemRectMax();
            ImGui::OpenPopup("month_popup");
        }
        ImGui::SameLine();
        ImGui::Text(".");
        ImGui::SameLine();
        if(ImGui::Button(yearStr)) {
            pos_min = ImGui::GetItemRectMin();
            pos_max = ImGui::GetItemRectMax();
            ImGui::OpenPopup("year_popup"); 
        }

        ImGui::PopStyleVar();   //restore original item spacing
        ImGui::PopStyleColor(); //restore original button color

        ImGuiTableFlags table_flags = ImGuiTableFlags_ScrollY;
        ImVec2 tableHeight = ImVec2(0.0f, ImGui::GetTextLineHeight() * 5);

        size_t daysInMonth = daysPerMonth[selectedMonthIndex];      //set max days per month
        //leap year check for max days in February
        if ((selectedYearIndex + 2) % 4 == 0 && selectedMonthIndex == 1) {  //+2 because year array starts in 2026, 2028 is next leap year
            daysInMonth = 29;
        }

        //shift Popup so, that the number on the button and the on the appearing popup align
        //TODO: convert magic numbers to calculations
        ImVec2 pivotDay{0.33, 0.5};
        ImVec2 pivotMonth{0.34, 0.5};
        ImVec2 pivotYear{0.38, 0.5};

        //edge cases for start and end of the scrollable range
        switch(selectedDayIndex) {
            case 0:  pivotDay = ImVec2(0.33, 0.18); break;  //1
            case 1:  pivotDay = ImVec2(0.33, 0.39); break;  //2
            case dayArrayLength - 5: pivotDay = (daysInMonth == 28) ? ImVec2(0.33, 0.60) : ImVec2(0.33, 0.5); break;  //27
            case dayArrayLength - 4: pivotDay = (daysInMonth == 28) ? ImVec2(0.33, 0.81) : ((daysInMonth == 29) ? ImVec2(0.33, 0.60) :  ImVec2(0.33, 0.5)); break;  //28
            case dayArrayLength - 3: pivotDay = (daysInMonth == 29) ? ImVec2(0.33, 0.81) : ((daysInMonth == 30) ? ImVec2(0.33, 0.60) : ImVec2(0.33, 0.50)); break;  //29
            case dayArrayLength - 2: pivotDay = (daysInMonth == 30) ? ImVec2(0.33, 0.81) : ImVec2(0.33, 0.60); break;  //30
            case dayArrayLength - 1: pivotDay = ImVec2(0.33, 0.81); break;  //31
        }
        switch(selectedMonthIndex) {
            case 0:  pivotMonth = ImVec2(0.34, 0.18); break;    //Jan
            case 1:  pivotMonth = ImVec2(0.34, 0.39); break;    //Feb
            case monthArrayLength - 2: pivotMonth = ImVec2(0.34, 0.60); break;    //Nov
            case monthArrayLength - 1: pivotMonth = ImVec2(0.34, 0.81); break;    //Dec
        }
        switch(selectedYearIndex) {
            case 0:  pivotYear = ImVec2(0.38, 0.18); break; //first year
            case 1:  pivotYear = ImVec2(0.38, 0.39); break;
            case yearArrayLength - 2: pivotYear = ImVec2(0.38, 0.60); break;
            case yearArrayLength - 1: pivotYear = ImVec2(0.38, 0.81); break;    //last year
        }

        ImGui::SetNextWindowPos(ImVec2((pos_max.x + pos_min.x) / 2.0f, (pos_max.y + pos_min.y) / 2.0f), 0, pivotDay);
        if (ImGui::BeginPopup("day_popup")) {
            if (ImGui::BeginTable("daySelection", 1, table_flags, tableHeight)) {
                
                for (size_t day = 0; day < daysInMonth; ++day) {
                    ImGui::TableNextRow();
                    ImGui::TableNextColumn();
                    bool isSelected = (selectedDayIndex == day);
                    if(ImGui::Selectable(dayArray[day], isSelected)) { 
                        selectedDayIndex = day; 
                        ImGui::CloseCurrentPopup();
                    }
                    
                    if(ImGui::IsWindowAppearing() && isSelected) { ImGui::SetScrollHereY(); }
                }
                ImGui::EndTable();
            }
            ImGui::EndPopup();
        }
        
        ImGui::SetNextWindowPos(ImVec2((pos_max.x + pos_min.x) / 2.0f, (pos_max.y + pos_min.y) / 2.0f), 0, pivotMonth);
        if (ImGui::BeginPopup("month_popup")) {
            if (ImGui::BeginTable("monthSelection", 1, table_flags, tableHeight)) {
                for (size_t month = 0; month < 12; ++month) {
                    ImGui::TableNextRow();
                    ImGui::TableNextColumn();
                    bool isSelected = (selectedMonthIndex == month);
                    if(ImGui::Selectable(monthArray[month], isSelected)) { 
                        selectedMonthIndex = month; 
                        ImGui::CloseCurrentPopup();
                    }
                    
                    if(ImGui::IsWindowAppearing() && isSelected) { ImGui::SetScrollHereY(); }
                }
                ImGui::EndTable();
            }
            ImGui::EndPopup();
        }
        
        ImGui::SetNextWindowPos(ImVec2((pos_max.x + pos_min.x) / 2.0f, (pos_max.y + pos_min.y) / 2.0f), 0, pivotYear);
        if (ImGui::BeginPopup("year_popup")) {
            if (ImGui::BeginTable("yearSelection", 1, table_flags, tableHeight)) {
                for (size_t year = 0; year < 60; ++year) {
                    ImGui::TableNextRow();
                    ImGui::TableNextColumn();
                    bool isSelected = (selectedYearIndex == year);
                    if(ImGui::Selectable(yearArray[year], isSelected)) { 
                        selectedYearIndex = year; 
                        ImGui::CloseCurrentPopup();
                    }

                    if(ImGui::IsWindowAppearing() && isSelected) { ImGui::SetScrollHereY(); }
                }
                ImGui::EndTable();
            }
            ImGui::EndPopup();
        }
        
        ImGui::PopID(); //ID of this time picker widget

        const auto selectedDate = std::chrono::year{static_cast<int>(selectedYearIndex + 2026)} / std::chrono::month{static_cast<unsigned>(selectedMonthIndex + 1)} / std::chrono::day{static_cast<unsigned>(selectedDayIndex + 1)};

        //reconstruct the modified timepoint from date and daytime
        *time = sys_days{selectedDate} + dayTime.to_duration();

        return true;
    }

}