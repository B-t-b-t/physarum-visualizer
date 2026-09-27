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
}