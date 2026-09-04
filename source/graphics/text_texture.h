#ifndef TEXT_TEXTURE_H
#define TEXT_TEXTURE_H

#include <string>
#include <vector>

#include "imgui.h"

#include "framebuffer.h"
#include "shader_program.h"
#include "texture.h"
#include "../application_state.h"
#include "../utility/fileHandling.h"
#include "font_atlas.h"

struct Vertex {
    ImVec2 position;
    float depth{0.0f};
    ImVec4 color;
    ImVec2 texCoord;
};

// Represents text via a texture, which is created by it's own rendering pipeline.
// Pipeline: Vert Shader -> Frag Shader -> Texture (attached to a Framebuffer)
// Using a texture instead of direct screen rendering, because the simulation needs access to it.
class TextTexture : public Texture {

public:

    TextTexture() = default;
    TextTexture(int width, int height, ApplicationState* appState);
    TextTexture(const TextTexture&) = delete;   //avoid copying
    TextTexture& operator=(const TextTexture&) = delete;
    TextTexture(TextTexture&&);
    TextTexture& operator=(TextTexture&&);
    ~TextTexture();

    void createTexture(std::string text, FontAtlas& fontAtlas);
    void textureToFile();

private:

    static ShaderProgram textRenderProgram_;
    static bool isShaderProgramInitialized_;
    
    FrameBuffer outputFrameBuffer_;

    ApplicationState* appState_{nullptr};

    std::vector<Vertex> quadVertices_{};
    unsigned int m_VAO;
	unsigned int m_VBO;
};

#endif // TEXT_TEXTURE_H