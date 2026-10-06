#ifndef DUAL_LISTBOX_H
#define DUAL_LISTBOX_H

#include <map>
#include <string>
#include <utility>
#include <vector>

#include "imgui.h"
#include "imgui_internal.h"

class DualListBox {
public:
    template <typename Preset, typename Predicate, typename Label>
    void reset(const std::map<std::string, Preset>& presets,
               const std::map<std::string, Preset*>& activePresets,
               Predicate matchesPresetType,
               Label makeLabel) {
        for (auto& items : items_) {
            items.clear();
        }

        for (auto& selection : selections_) {
            selection.Clear();
        }

        for (const auto& [key, preset] : presets) {
            if (!matchesPresetType(preset)) {
                continue;
            }

            const int side = activePresets.contains(key) ? 1 : 0;
            items_[side].push_back(Item{
                key,
                makeLabel(key, preset)
            });
        }

        sortItems(0);
        sortItems(1);
    }

    std::vector<std::string> getActivePresetKeys() const;

    void show(const char* id);

private:
    struct Item {
        std::string key;
        std::string label;
    };

    void moveAll(int src, int dst);
    void moveSelected(int src, int dst);
    void applySelectionRequests(ImGuiMultiSelectIO* msIo, int side);
    void sortItems(int side);

    static ImGuiID itemId(const Item& item);

    std::vector<Item> items_[2];
    ImGuiSelectionBasicStorage selections_[2];
};

#endif // DUAL_LISTBOX_H