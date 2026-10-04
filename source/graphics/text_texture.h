#ifndef TEXT_TEXTURE_H
#define TEXT_TEXTURE_H

#include <filesystem>                 // for std::filesystem::path
#include <string>
#include <vector>

#include "framebuffer.h"
#include "shader_program.h"
#include "texture.h"
#include "../application_state.h"
#include "../utility/fileHandling.h"
#include "../utility/filepaths.h"
#include "../utility/vector_math.h"  // for phys::Vec2, phys::Vec4
#include "font_atlas.h"

struct Vertex {
    phys::Vec2<float> position;
    float depth{0.0f};
    phys::Vec4<float> color;
    phys::Vec2<float> texCoord;
};

// Represents text via a texture, which is created by it's own rendering pipeline.
// Pipeline: Vert Shader -> Frag Shader -> Texture (attached to a Framebuffer)
// Using a texture instead of direct screen rendering, because the simulation needs access to it.
class TextTexture : public Texture {

public:

    TextTexture() = default;
    TextTexture(int width, int height, FilePaths* paths, ApplicationState* appState);
    TextTexture(const TextTexture&) = delete;   //avoid copying
    TextTexture& operator=(const TextTexture&) = delete;
    TextTexture(TextTexture&&);
    TextTexture& operator=(TextTexture&&);
    ~TextTexture();

    void createTexture(std::string& text, FontAtlas& fontAtlas);
    void textureToFile(std::filesystem::path filePath);

private:

    static ShaderProgram textRenderProgram_;
    static bool isShaderProgramInitialized_;
    
    FrameBuffer outputFrameBuffer_;

    ApplicationState* appState_{nullptr};

    std::vector<Vertex> quadVertices_{};
    unsigned int m_VAO{0};
	unsigned int m_VBO{0};
};

#endif // TEXT_TEXTURE_H