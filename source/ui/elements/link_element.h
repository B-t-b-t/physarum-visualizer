#ifndef LINK_ELEMENT_H
#define LINK_ELEMENT_H

namespace ImGui {
    bool link(const char* id, bool* isLinked, int elementCount = 2, float thickness = 1.0f, float capLength = 7.0f);
}

#endif // LINK_ELEMENT_H