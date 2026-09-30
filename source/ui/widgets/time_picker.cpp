#include "time_picker.h"

#include <array>
#include <cmath>
#include <iostream>

#include "imgui.h"

namespace ImGui {

    bool timePicker(const char* ID, std::chrono::system_clock::time_point* time) {
        using namespace std::chrono;
        
        ImGui::PushID(ID);
        //split into date and daytime components
        year_month_day date = year_month_day{floor<days>(*time)};
        hh_mm_ss<seconds> dayTime = hh_mm_ss{floor<seconds>(*time) - floor<days>(*time)};

        //array for string display of digits 00-59
        static std::array<const char*, 60> numArray = {"00", "01", "02", "03", "04", "05", "06", "07", "08", "09",
                                              "10", "11", "12", "13", "14", "15", "16", "17", "18", "19",
                                              "20", "21", "22", "23", "24", "25", "26", "27", "28", "29",
                                              "30", "31", "32", "33", "34", "35", "36", "37", "38", "39",
                                              "40", "41", "42", "43", "44", "45", "46", "47", "48", "49",
                                              "50", "51", "52", "53", "54", "55", "56", "57", "58", "59"};

        size_t selectedHour = (size_t)(dayTime.hours().count());
        size_t selectedMinute = (size_t)(dayTime.minutes().count());
        size_t selectedSecond = (size_t)(dayTime.seconds().count());

        static size_t selectedArrayLength = 0;
        static size_t selectedArrayIndex = 0;

        char hourStr[12];   //can hold length of e.g. "00##second" and null terminator
        char minuteStr[12];
        char secondStr[12];
        sprintf(hourStr, "%s##hour", numArray[selectedHour]);
        sprintf(minuteStr, "%s##minute", numArray[selectedMinute]);
        sprintf(secondStr, "%s##second", numArray[selectedSecond]);

        static ImVec2 pos_min{};
        static ImVec2 pos_max{};

        static ImVec2 popupShift{0, 0}; //for shifting the popup, as if the button is visually being extended to a selection box after clicked

        //============ Buttons Hour, Minute, Second ============

        //save states between button and selection clicks
        static enum class Clicked { None, Hour, Minute, Second } buttonClicked = Clicked::None;

        ImGui::PushStyleColor(ImGuiCol_Button, ImGuiCol_WindowBg);
        //reduce space between buttons for a better look
        ImGui::PushStyleVarX(ImGuiStyleVar_ItemSpacing, (float)(int)(ImGui::GetStyle().ItemSpacing.x * 0.1));

        if(ImGui::Button(hourStr)) {
            pos_min = ImGui::GetItemRectMin();
            pos_max = ImGui::GetItemRectMax();
            selectedArrayIndex = selectedHour;
            selectedArrayLength = 24; //hours range from 0 to 23
            popupShift = ImVec2(0, 0); //reset popup shift when opening a new popup
            buttonClicked = Clicked::Hour;
            ImGui::OpenPopup("timeSelection"); 
        }

        ImGui::SameLine();
        ImGui::Text(":");
        ImGui::SameLine();

        if(ImGui::Button(minuteStr)) {
            pos_min = ImGui::GetItemRectMin();
            pos_max = ImGui::GetItemRectMax();
            selectedArrayIndex = selectedMinute;
            selectedArrayLength = 60; //minutes range from 0 to 59
            popupShift = ImVec2(0, 0); //reset popup shift when opening a new popup
            buttonClicked = Clicked::Minute;
            ImGui::OpenPopup("timeSelection");
        }

        ImGui::SameLine();
        ImGui::Text(":");
        ImGui::SameLine();

        if(ImGui::Button(secondStr)) {
            pos_min = ImGui::GetItemRectMin();
            pos_max = ImGui::GetItemRectMax();
            selectedArrayIndex = selectedSecond;
            selectedArrayLength = 60; //seconds range from 0 to 59
            popupShift = ImVec2(0, 0); //reset popup shift when opening a new popup
            buttonClicked = Clicked::Second;
            ImGui::OpenPopup("timeSelection");
        }

        ImGui::PopStyleVar();   //restore original item spacing
        ImGui::PopStyleColor(); //restore original button color

        //============ End Buttons ============

        ImGuiTableFlags table_flags = ImGuiTableFlags_ScrollY | ImGuiTableFlags_BordersOuterV;
        ImVec2 tableHeight = ImVec2(0.0f, ImGui::GetTextLineHeight() * 5);

        //calculate middle of clicked button for later popup shift
        ImVec2 clickedButtonPos{(pos_min.x + pos_max.x) / 2.0f, (pos_min.y + pos_max.y) / 2.0f};

        int numElements = 0;    //number of elements to shift the popup by (for elements at the beginning or end of the table)
        if(selectedArrayIndex == 0) { numElements = 3; }
        else if(selectedArrayIndex == 1) { numElements = 1; }
        else if(selectedArrayIndex == selectedArrayLength - 1) { numElements = -1; }
        else if(selectedArrayIndex == selectedArrayLength) { numElements = -3; }

        if(numElements != 0) {
            popupShift.y = (numElements * (ImGui::GetTextLineHeight()) + 2 * numElements * ImGui::GetStyle().CellPadding.y) / 2.0f;
        }

        bool timeChanged = false;

        //============ Popup logic ============

        ImGui::PushStyleVar(ImGuiStyleVar_ScrollbarSize, 0);
        ImGui::PushStyleVar(ImGuiStyleVar_PopupBorderSize, 0);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(2, 0));
        ImGui::PushStyleColor(ImGuiCol_PopupBg, ImVec4(0.15f, 0.15f, 0.15f, 1.00f));

        //shift popup
        ImGui::SetNextWindowPos(ImVec2(clickedButtonPos.x + popupShift.x, clickedButtonPos.y + popupShift.y), 0, ImVec2{0.5, 0.5});
        if (ImGui::BeginPopup("timeSelection")) {

            if (ImGui::BeginTable("selectionTable", 1, table_flags, tableHeight)) {
                for (size_t i = 0; i < selectedArrayLength; ++i) {
                    ImGui::TableNextRow();
                    ImGui::TableNextColumn();
                    bool isSelected = (selectedArrayIndex == i);
                    if(ImGui::Selectable(numArray[i], isSelected)) {
                        selectedArrayIndex = i;
                        switch (buttonClicked) {
                            case Clicked::Hour: selectedHour = selectedArrayIndex; break;
                            case Clicked::Minute: selectedMinute = selectedArrayIndex; break;
                            case Clicked::Second: selectedSecond = selectedArrayIndex; break;
                            default: break;
                        }

                        const auto selectedDayTime = std::chrono::hh_mm_ss<std::chrono::seconds>{std::chrono::hours(selectedHour) + std::chrono::minutes(selectedMinute) + std::chrono::seconds(selectedSecond)};

                        //reconstruct the modified timepoint from date and daytime
                        *time = sys_days{date} + selectedDayTime.to_duration();

                        timeChanged = true;
                        ImGui::CloseCurrentPopup();
                    }

                    if(ImGui::IsWindowAppearing() && isSelected) { ImGui::SetScrollHereY(); }
                }
                ImGui::EndTable();
            }
            ImGui::EndPopup();
        }

        ImGui::PopStyleVar(3);
        ImGui::PopStyleColor();

        ImGui::PopID(); //ID of this dayTime picker widget

        return timeChanged;
    }


    bool datePicker(const char* ID, std::chrono::system_clock::time_point* time) {
        using namespace std::chrono;
        ImGui::PushID(ID);

        //split into date and daytime components
        year_month_day date = year_month_day{floor<days>(*time)};
        hh_mm_ss<seconds> dayTime = hh_mm_ss{floor<seconds>(*time) - floor<days>(*time)};
        size_t selectedDayIndex = (size_t)static_cast<unsigned int>(date.day()) - 1;
        size_t selectedMonthIndex = (size_t)static_cast<unsigned int>(date.month()) - 1;
        size_t selectedYearIndex = (size_t)static_cast<int>(date.year()) - 2026;

        //arrays for string display of days, months, and years
        static std::array<const char*, 31> dayArray = {"01", "02", "03", "04", "05", "06", "07", "08", "09", "10",
            "11", "12", "13", "14", "15", "16", "17", "18", "19", "20",
            "21", "22", "23", "24", "25", "26", "27", "28", "29", "30", "31"};
        
        static std::array<const char*, 12> monthArray = {"Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};
        static std::array<const size_t, 12> daysPerMonth = {   31,    28,    31,    30,    31,    30,    31,    31,    30,    31,    30,    31};

        static std::array<const char*, 60> yearArray = {"2026", "2027", "2028", "2029", "2030", "2031", "2032", "2033", "2034", "2035",
                                                             "2036", "2037", "2038", "2039", "2040", "2041", "2042", "2043", "2044", "2045",
                                                             "2046", "2047", "2048", "2049", "2050", "2051", "2052", "2053", "2054", "2055",
                                                             "2056", "2057", "2058", "2059", "2060", "2061", "2062", "2063", "2064", "2065",
                                                             "2066", "2067", "2068", "2069", "2070", "2071", "2072", "2073", "2074", "2075",
                                                             "2076", "2077", "2078", "2079", "2080", "2081", "2082", "2083", "2084", "2085"};
        
        //calculate just once for positioning of the month popup
        static ImVec2 maxMonthTextWidth{0.0f, 0.0f};
        if(!(maxMonthTextWidth.x > 0.0f) && ImGui::IsWindowAppearing()) {
            for(const char* month : monthArray) {
                ImVec2 textWidth = ImGui::CalcTextSize(month);
                if(textWidth.x > maxMonthTextWidth.x) {
                    maxMonthTextWidth = textWidth;
                }
            }
        }

        //for display of button names
        char dayBtnName[12];    //can hold length of e.g. "00##second" and null terminator
        char monthBtnName[12];
        char yearBtnName[12];
        sprintf(dayBtnName, "%s##day", dayArray[selectedDayIndex]);
        sprintf(monthBtnName, "%s##month", monthArray[selectedMonthIndex]);
        sprintf(yearBtnName, "%s##year", yearArray[selectedYearIndex]);
        
        static ImVec2 pos_min{};
        static ImVec2 pos_max{};
        
        static ImVec2 popupShift{0, 0}; //for shifting the popup, as if the button is visually being extended to a selection box after clicked

        static const char** selectedArray = nullptr;
        static size_t selectedArrayLength = 0;
        static size_t selectedArrayIndex = 0;

        //============ Buttons Day, Month, Year ============

        //save states between button and selection clicks
        static enum class Clicked { None, Day, Month, Year } buttonClicked = Clicked::None;

        ImGui::PushStyleColor(ImGuiCol_Button, ImGuiCol_WindowBg);
        ImGui::PushStyleVarX(ImGuiStyleVar_ItemSpacing, (float)(int)(ImGui::GetStyle().ItemSpacing.x * 0.1));
        
        if(ImGui::Button(dayBtnName)) {
            pos_min = ImGui::GetItemRectMin();
            pos_max = ImGui::GetItemRectMax();

            selectedArrayIndex = selectedDayIndex;
            selectedArrayLength = daysPerMonth[selectedMonthIndex] - 1;      //set max days per month
            //leap year check for max days in February
            if ((selectedYearIndex) % 4 == 2 && selectedMonthIndex == 1) {  //==2 because leap years are on index 2*n
                selectedArrayLength = 28;
            }
            popupShift.x = 0;
            popupShift.y = 0;
            buttonClicked = Clicked::Day;
            selectedArray = dayArray.data();
            ImGui::OpenPopup("dateSelection");
        }

        ImGui::SameLine();
        ImGui::Text(".");
        ImGui::SameLine();

        if(ImGui::Button(monthBtnName)) {
            pos_min = ImGui::GetItemRectMin();
            pos_max = ImGui::GetItemRectMax();
            selectedArrayIndex = selectedMonthIndex;
            selectedArrayLength = monthArray.size() - 1;
            ImVec2 monthTextSize = ImGui::CalcTextSize(monthArray[selectedMonthIndex]);
            popupShift.x = (maxMonthTextWidth.x - monthTextSize.x) / 2.0f;
            popupShift.y = 0;
            buttonClicked = Clicked::Month;
            selectedArray = monthArray.data();
            ImGui::OpenPopup("dateSelection");
        }

        ImGui::SameLine();
        ImGui::Text(".");
        ImGui::SameLine();

        if(ImGui::Button(yearBtnName)) {
            pos_min = ImGui::GetItemRectMin();
            pos_max = ImGui::GetItemRectMax();
            selectedArrayIndex = selectedYearIndex;
            selectedArrayLength = yearArray.size() - 1;
            popupShift.x = 0;
            popupShift.y = 0;
            buttonClicked = Clicked::Year;
            selectedArray = yearArray.data();
            ImGui::OpenPopup("dateSelection");
        }

        ImGui::PopStyleVar();   //restore original item spacing
        ImGui::PopStyleColor(); //restore original button color

        //============ End Buttons ============

        //calculate middle of clicked button for later popup shift
        ImVec2 clickedButtonPos{(pos_min.x + pos_max.x) / 2.0f, (pos_min.y + pos_max.y) / 2.0f};

        int numElements = 0;    //number of elements to shift the popup by (for elements at the beginning or end of the table)
        if(selectedArrayIndex == 0) { numElements = 3; }
        else if(selectedArrayIndex == 1) { numElements = 1; }
        else if(selectedArrayIndex == selectedArrayLength - 1) { numElements = -1; }
        else if(selectedArrayIndex == selectedArrayLength) { numElements = -3; }

        if(numElements != 0) {
            popupShift.y = (numElements * (ImGui::GetTextLineHeight()) + 2 * numElements * ImGui::GetStyle().CellPadding.y) / 2.0f;
        }

        ImGuiTableFlags table_flags = ImGuiTableFlags_ScrollY | ImGuiTableFlags_BordersOuterV;
        ImVec2 tableHeight = ImVec2(0.0f, ImGui::GetTextLineHeight() * 5);

        //============ Popup logic ============

        ImGui::PushStyleVar(ImGuiStyleVar_ScrollbarSize, 0);
        ImGui::PushStyleVar(ImGuiStyleVar_PopupBorderSize, 0);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(2, 0));
        ImGui::PushStyleColor(ImGuiCol_PopupBg, ImVec4(0.15f, 0.15f, 0.15f, 1.00f));

        bool dateChanged = false;

        //shift popup
        ImGui::SetNextWindowPos(ImVec2(clickedButtonPos.x + popupShift.x, clickedButtonPos.y + popupShift.y), 0, ImVec2{0.5, 0.5});
        if (ImGui::BeginPopup("dateSelection")) {
            if (ImGui::BeginTable("selectionTable", 1, table_flags, tableHeight)) {
                
                for (size_t i = 0; i <= selectedArrayLength; ++i) {
                    ImGui::TableNextRow();
                    ImGui::TableNextColumn();
                    bool isSelected = (selectedArrayIndex == i);

                    if(ImGui::Selectable(selectedArray[i], isSelected)) {
                        selectedArrayIndex = i;
                        switch(buttonClicked) {
                            case Clicked::Day: selectedDayIndex = selectedArrayIndex; break;
                            case Clicked::Month: selectedMonthIndex = selectedArrayIndex; break;
                            case Clicked::Year: selectedYearIndex = selectedArrayIndex; break;
                            default: break;
                        }

                        const auto selectedDate = std::chrono::year{static_cast<int>(selectedYearIndex + 2026)} / std::chrono::month{static_cast<unsigned>(selectedMonthIndex + 1)} / std::chrono::day{static_cast<unsigned>(selectedDayIndex + 1)};
                        
                        //reconstruct the modified timepoint from date and daytime
                        *time = sys_days{selectedDate} + dayTime.to_duration();
                        dateChanged = true;

                        ImGui::CloseCurrentPopup();
                    }
                    
                    if(ImGui::IsWindowAppearing() && isSelected) { ImGui::SetScrollHereY(); }

                }
                ImGui::EndTable();
            }
            ImGui::EndPopup();
        }

        ImGui::PopStyleColor();
        ImGui::PopStyleVar(3);
        
        ImGui::PopID(); //ID of this time picker widget

        return dateChanged;
    }

}