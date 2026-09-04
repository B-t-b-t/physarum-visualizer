#include "text_texture.h"

#include "SDL3/SDL.h"
#include "imgui.h"

#include "../utility/fileHandling.h"

ShaderProgram TextTexture::textRenderProgram_{};
bool TextTexture::isShaderProgramInitialized_ = false;

TextTexture::TextTexture(int width, int height, ApplicationState* appState)
 : Texture(TextureProperties{.width = width, .height = height, .wrapX = TextureWrap::CLAMP_TO_EDGE, .wrapY = TextureWrap::CLAMP_TO_EDGE}),
   outputFrameBuffer_(),
   appState_(appState)
{
    //share shader program among all instances because it is expensive
    if(!isShaderProgramInitialized_) {
        Shader vertexShader = Shader("./res/text_vertex.vs", ShaderType::VERTEX_SHADER);
        Shader fragmentShader = Shader("./res/text_fragment.fs", ShaderType::FRAGMENT_SHADER);
        textRenderProgram_ = ShaderProgram("TextRenderPipeline", {&vertexShader, &fragmentShader});
        isShaderProgramInitialized_ = true;
    }

    outputFrameBuffer_.attachTexture(GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, getID(), 0);

    //create VAO and VBO for later use
    glGenVertexArrays(1, &m_VAO);
    glBindVertexArray(m_VAO);
    
    glGenBuffers(1, &m_VBO);
    glBindBuffer(GL_ARRAY_BUFFER, m_VBO);
    
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, position));
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, color));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, texCoord));
    glEnableVertexAttribArray(2);

    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

TextTexture::TextTexture(TextTexture&& other) 
 : Texture(std::move(other)),
   outputFrameBuffer_(std::move(other.outputFrameBuffer_)),
   appState_(other.appState_),
   quadVertices_(std::move(other.quadVertices_)),
   m_VAO(other.m_VAO),
   m_VBO(other.m_VBO)
{
    other.appState_ = nullptr;
    other.m_VAO = 0;
    other.m_VBO = 0;
}

TextTexture& TextTexture::operator=(TextTexture&& rhs) {
    if (this != &rhs) {
        Texture::operator=(std::move(rhs));
        outputFrameBuffer_ = std::move(rhs.outputFrameBuffer_);
        appState_ = rhs.appState_;
        quadVertices_ = std::move(rhs.quadVertices_);
        m_VAO = rhs.m_VAO;
        m_VBO = rhs.m_VBO;

        rhs.appState_ = nullptr;
        rhs.m_VAO = 0;
        rhs.m_VBO = 0;
    }
    return *this;
}

TextTexture::~TextTexture() {
    // VAO = 0 is ignored by glDeleteVertexArrays, but check anyway
    if (m_VAO != 0) {
        glDeleteVertexArrays(1, &m_VAO);
    }
    // VBO = 0 is ignored by glDeleteBuffers, but check anyway
    if (m_VBO != 0) {
        glDeleteBuffers(1, &m_VBO);
    }
}

void TextTexture::createTexture(std::string& text, FontAtlas& fontAtlas) {
    quadVertices_.clear();
    quadVertices_.reserve(text.size() * 6); //6 vertices per character (2 triangles)

    //scale to render the text in
    float textScale = 5.0f;
    //position of the text in normalized coordinates (x = [-1, 1] y = [-1, 1])
    ImVec2 position = ImVec2(0.0f, 0.0f);    //center
    //size of a pixel in normalized coordinates (x = [-1, 1] y = [-1, 1])
    ImVec2 pixelSize = ImVec2(2.0f / appState_->universalShaderSettings.windowWidth, 
                              2.0f / appState_->universalShaderSettings.windowHeight);
    //order of vertices to render a quad as two triangles
    int order[6] = {0, 1, 2, 0, 2, 3};
    //white text color
    ImVec4 color = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);

    //shift text position origin so that text is in the middle of the texture
    ImVec2 textDimensions = ImVec2(0.0f, 0.0f);

    std::vector<FontCharInfo>& fontCharInfos = fontAtlas.getFontCharInfos();
    int firstChar = fontAtlas.getFirstChar();

    for(const char c : text) {
        FontCharInfo charInfo = fontCharInfos[(size_t)(c - firstChar)];

        textDimensions.x += charInfo.xadvance * pixelSize.x * textScale;
        textDimensions.y = std::max(textDimensions.y, (charInfo.y1 - charInfo.y0) * pixelSize.y * textScale);
    }
    position.x -= textDimensions.x / 2.0f;
    position.y -= textDimensions.y / 2.0f;

    //create quad vertices and texture coordinates for each character in the text
    for(const char c : text) {
        FontCharInfo charInfo = fontCharInfos[(size_t)(c - firstChar)];

        //width and height of the glyph bounding box in normalized coordinates (x = [-1, 1] y = [-1, 1])
        ImVec2 glyphSize = ImVec2(charInfo.sizeX * pixelSize.x * textScale, 
                                  charInfo.sizeY * pixelSize.y * textScale);
        //position of the bottom left corner of the glyph bounding box in normalized coordinates (x = [-1, 1] y = [-1, 1])
        ImVec2 glyphBoundingBoxBottomLeft = ImVec2(position.x + (charInfo.xoff * pixelSize.x * textScale), 
                                                   position.y - (charInfo.yoff + charInfo.y1 - charInfo.y0) * pixelSize.y * textScale);


        //the order of vertices of a quad goes top-right, top-left, bottom-left, bottom-right
        ImVec2 glyphVertices[4] = 
        {
            { glyphBoundingBoxBottomLeft.x + glyphSize.x, glyphBoundingBoxBottomLeft.y + glyphSize.y },
            { glyphBoundingBoxBottomLeft.x, glyphBoundingBoxBottomLeft.y + glyphSize.y },
            { glyphBoundingBoxBottomLeft.x, glyphBoundingBoxBottomLeft.y },
            { glyphBoundingBoxBottomLeft.x + glyphSize.x, glyphBoundingBoxBottomLeft.y }
        };

        ImVec2 glyphTextureCoords[4] = 
        {
            { charInfo.s1, charInfo.t0 },
            { charInfo.s0, charInfo.t0 },
            { charInfo.s0, charInfo.t1 },
            { charInfo.s1, charInfo.t1 },
        };

        // order = [0, 1, 2, 0, 2, 3] is meant to represent 2 triangles: 
        // one by glyphVertices[0], glyphVertices[1], glyphVertices[2] and one by glyphVertices[0], glyphVertices[2], glyphVertices[3]
        for(int i = 0; i < 6; i++) {
            quadVertices_.push_back(Vertex(glyphVertices[order[i]],
                0.0f,
                color,
                glyphTextureCoords[order[i]]
            ));
        }

        //advance the position for the next character
        position.x += charInfo.xadvance * pixelSize.x * textScale;
    }

    //send quads on GPU
    glBindVertexArray(m_VAO);
    glBindBuffer(GL_ARRAY_BUFFER, m_VBO);
    
	glBufferData(GL_ARRAY_BUFFER, static_cast<GLsizeiptr>(quadVertices_.size() * sizeof(Vertex)), &quadVertices_.front(), GL_STATIC_DRAW);
    
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, fontAtlas.getTextureID());

    textRenderProgram_.use();

    const GLint fontAtlasLocation = glGetUniformLocation(
        textRenderProgram_.getProgramID(),
        "fontAtlas"
    );

    if(fontAtlasLocation >= 0) {
        glUniform1i(fontAtlasLocation, 0);
    }

    GLint previousViewport[4];
    glGetIntegerv(GL_VIEWPORT, previousViewport);

    outputFrameBuffer_.bind();
    glViewport(0, 0, getWidth(), getHeight());
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    //draw
    glBindVertexArray(m_VAO);
	glDrawArrays(GL_TRIANGLES, 0, quadVertices_.size());

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, 0);
    outputFrameBuffer_.unbind();

    glViewport(previousViewport[0], previousViewport[1], previousViewport[2], previousViewport[3]);

}

void TextTexture::textureToFile() {
    const int width = getWidth();
    const int height = getHeight();

    std::vector<float> textureData(static_cast<size_t>(width * height * 4));
    std::vector<uint8_t> imageData(static_cast<size_t>(width * height * 4));

    glBindTexture(GL_TEXTURE_2D, getID());
    glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_FLOAT, textureData.data());
    glBindTexture(GL_TEXTURE_2D, 0);

    const GLenum error = glGetError();
    if(error != GL_NO_ERROR) {
        SDL_LogError(
            SDL_LOG_CATEGORY_APPLICATION,
            "glGetTexImage failed: 0x%x",
            static_cast<unsigned int>(error)
        );
        return;
    }

    for(size_t i = 0; i < textureData.size(); ++i) {
        const float channel = std::clamp(textureData[i], 0.0f, 1.0f);
        imageData[i] = static_cast<uint8_t>(channel * 255.0f);
    }

    SDL_Surface* surface = SDL_CreateSurfaceFrom(
        width,
        height,
        SDL_PIXELFORMAT_RGBA32,
        imageData.data(),
        width * 4
    );

    if(surface == nullptr) {
        SDL_LogError(
            SDL_LOG_CATEGORY_APPLICATION,
            "Failed to create texture export surface: %s",
            SDL_GetError()
        );
        return;
    }

    saveImageToFile(surface, "./", "textTexture", ".png", true);
    SDL_DestroySurface(surface);
}