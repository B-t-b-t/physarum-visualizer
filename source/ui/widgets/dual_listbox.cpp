#include "dual_listbox.h"

#include <algorithm>
#include <cfloat>
#include <cstddef>

std::vector<std::string> DualListBox::getActivePresetKeys() const {
    std::vector<std::string> keys;
    keys.reserve(items_[1].size());

    for (const Item& item : items_[1]) {
        keys.push_back(item.key);
    }

    return keys;
}

ImGuiID DualListBox::itemId(const Item& item) {
    return ImHashStr(item.key.c_str());
}

void DualListBox::moveAll(int src, int dst) {
    IM_ASSERT((src == 0 && dst == 1) || (src == 1 && dst == 0));

    for (const Item& item : items_[src]) {
        items_[dst].push_back(item);
    }

    items_[src].clear();
    sortItems(dst);

    selections_[src].Clear();
    selections_[dst].Clear();
}

void DualListBox::moveSelected(int src, int dst) {
    for (auto item = items_[src].begin(); item != items_[src].end();) {
        if (!selections_[src].Contains(itemId(*item))) {
            ++item;
            continue;
        }

        items_[dst].push_back(*item);
        item = items_[src].erase(item);
    }

    sortItems(dst);

    selections_[src].Clear();
    selections_[dst].Clear();
}

void DualListBox::applySelectionRequests(ImGuiMultiSelectIO* msIo, int side) {
    selections_[side].UserData = &items_[side];
    selections_[side].AdapterIndexToStorageId =
        [](ImGuiSelectionBasicStorage* self, int index) {
            const auto* items =
                static_cast<const std::vector<Item>*>(self->UserData);

            return itemId((*items)[static_cast<std::size_t>(index)]);
        };

    selections_[side].ApplyRequests(msIo);
}

void DualListBox::sortItems(int side) {
    std::sort(
        items_[side].begin(),
        items_[side].end(),
        [](const Item& left, const Item& right) {
            if (left.label == right.label) {
                return left.key < right.key;
            }

            return left.label < right.label;
        });
}

void DualListBox::show(const char* id) {
    ImGui::PushID(id);

    if (ImGui::BeginTable("split", 3, ImGuiTableFlags_None)) {

        ImGui::TableSetupColumn("", ImGuiTableColumnFlags_WidthStretch);
        ImGui::TableSetupColumn("", ImGuiTableColumnFlags_WidthFixed);
        ImGui::TableSetupColumn("", ImGuiTableColumnFlags_WidthStretch);
        ImGui::TableNextRow();

        int requestMoveSelected = -1;
        int requestMoveAll = -1;
        float leftChildHeight = 0.0f;

        ImGui::TableSetColumnIndex(0);
        ImGui::Text("Inactive (%d)", static_cast<int>(items_[0].size()));

        const float itemHeight = ImGui::GetTextLineHeightWithSpacing();

        ImGui::SetNextWindowContentSize(
            ImVec2(0.0f, items_[0].size() * itemHeight));

        ImGui::SetNextWindowSizeConstraints(
            ImVec2(0.0f, ImGui::GetFrameHeightWithSpacing() * 4),
            ImVec2(FLT_MAX, FLT_MAX));

        if (ImGui::BeginChild(
                "Left",
                ImVec2(-FLT_MIN, ImGui::GetFontSize() * 20),
                ImGuiChildFlags_FrameStyle | ImGuiChildFlags_ResizeY)) {

            ImGuiSelectionBasicStorage& selection = selections_[0];
            leftChildHeight = ImGui::GetWindowSize().y;

            ImGuiMultiSelectIO* msIo = ImGui::BeginMultiSelect(
                ImGuiMultiSelectFlags_BoxSelect1d,
                selection.Size,
                static_cast<int>(items_[0].size()));

            applySelectionRequests(msIo, 0);

            for (int itemIndex = 0;
                 itemIndex < static_cast<int>(items_[0].size());
                 ++itemIndex) {

                const Item& item =
                    items_[0][static_cast<std::size_t>(itemIndex)];

                ImGui::SetNextItemSelectionUserData(itemIndex);
                ImGui::Selectable(
                    item.label.c_str(),
                    selection.Contains(itemId(item)),
                    ImGuiSelectableFlags_AllowDoubleClick);

                if (ImGui::IsItemFocused() &&
                    (ImGui::IsKeyPressed(ImGuiKey_Enter) ||
                     ImGui::IsKeyPressed(ImGuiKey_KeypadEnter) ||
                     ImGui::IsMouseDoubleClicked(0))) {
                    requestMoveSelected = 0;
                }
            }

            msIo = ImGui::EndMultiSelect();
            applySelectionRequests(msIo, 0);

            ImGui::EndChild();
        }

        ImGui::TableSetColumnIndex(2);
        ImGui::Text("Active (%d)", static_cast<int>(items_[1].size()));

        if (ImGui::BeginChild(
                "Right",
                ImVec2(-FLT_MIN, leftChildHeight),
                ImGuiChildFlags_FrameStyle)) {

            ImGuiSelectionBasicStorage& selection = selections_[1];

            ImGuiMultiSelectIO* msIo = ImGui::BeginMultiSelect(
                ImGuiMultiSelectFlags_BoxSelect1d,
                selection.Size,
                static_cast<int>(items_[1].size()));

            applySelectionRequests(msIo, 1);

            for (int itemIndex = 0;
                 itemIndex < static_cast<int>(items_[1].size());
                 ++itemIndex) {

                const Item& item =
                    items_[1][static_cast<std::size_t>(itemIndex)];

                ImGui::SetNextItemSelectionUserData(itemIndex);
                ImGui::Selectable(
                    item.label.c_str(),
                    selection.Contains(itemId(item)),
                    ImGuiSelectableFlags_AllowDoubleClick);

                if (ImGui::IsItemFocused() &&
                    (ImGui::IsKeyPressed(ImGuiKey_Enter) ||
                     ImGui::IsKeyPressed(ImGuiKey_KeypadEnter) ||
                     ImGui::IsMouseDoubleClicked(0))) {
                    requestMoveSelected = 1;
                }
            }

            msIo = ImGui::EndMultiSelect();
            applySelectionRequests(msIo, 1);

            ImGui::EndChild();
        }

        ImGui::TableSetColumnIndex(1);
        ImGui::NewLine();

        const ImVec2 buttonSize{
            ImGui::GetFrameHeight(),
            ImGui::GetFrameHeight()
        };

        //Fontawesome: fa-solid fa-angles-right 
        if (ImGui::Button("\uf101", buttonSize)) {
            requestMoveAll = 0;
        }
        //Fontawesome: fa-solid fa-angle-right 
        if (ImGui::Button("\uf054", buttonSize)) {
            requestMoveSelected = 0;
        }
        
        //Fontawesome: fa-solid fa-angle-left 
        if (ImGui::Button("\uf104", buttonSize)) {
            requestMoveSelected = 1;
        }
        //Fontawesome: fa-solid fa-angles-left 
        if (ImGui::Button("\uf100", buttonSize)) {
            requestMoveAll = 1;
        }

        if (requestMoveAll != -1) {
            moveAll(requestMoveAll, requestMoveAll ^ 1);
        }

        if (requestMoveSelected != -1) {
            moveSelected(requestMoveSelected, requestMoveSelected ^ 1);
        }

        ImGui::EndTable();
    }

    ImGui::PopID();
}