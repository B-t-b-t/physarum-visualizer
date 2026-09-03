#ifndef FONT_ATLAS_H
#define FONT_ATLAS_H

#include <string>
#include <vector>
#include "texture.h"

#include "../utility/fileHandling.h"

class FontAtlas {
public:
    FontAtlas(std::string fontFileName);

    std::vector<FontCharInfo>& getFontCharInfos() { return fontCharInfos_; }
    int getFirstChar() { return firstChar_; }
    int getNumberOfChars() { return numberOfChars_; }
    GLuint getTextureID() { return fontAtlas_.getID(); }

private:

    int firstChar_{0};
    int numberOfChars_{0};
    std::vector<FontCharInfo> fontCharInfos_{};

    Texture fontAtlas_;
};

#endif // FONT_ATLAS_H