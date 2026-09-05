#ifndef FONT_ATLAS_H
#define FONT_ATLAS_H

#include <string>                     // for string
#include <vector>                     // for vector

#include "../utility/fileHandling.h"  // for FontCharInfo
#include "GL/glew.h"                  // for GLuint
#include "texture.h"                  // for Texture

class FontAtlas {
public:
    FontAtlas(std::string fontFileName);

    std::vector<FontCharInfo>& getFontCharInfos() { return fontCharInfos_; }
    int getFirstChar() { return firstChar_; }
    int getNumberOfChars() { return numberOfChars_; }
    int getFontSize() { return fontSize_; }
    GLuint getTextureID() { return fontAtlas_.getID(); }

private:

    int firstChar_{0};
    int numberOfChars_{0};
    float fontSize_{0.0f};
    std::vector<FontCharInfo> fontCharInfos_{};

    Texture fontAtlas_;
};

#endif // FONT_ATLAS_H