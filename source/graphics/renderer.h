#ifndef RENDERER_H
#define RENDERER_H

#include "../canvas.h"
#include "shader.h"
#include "shader_program.h"
#include "texture.h"
#include "bloom.h"

class Renderer {
public:

    Renderer();

    Renderer(const Renderer&) = delete;   //no copies, to prevent multiple destructor calls on same GL resources
    Renderer& operator=(const Renderer&) = delete;
    Renderer(Renderer&& rhs) noexcept;
    Renderer& operator=(Renderer&& rhs) noexcept;

    void draw();
    void clear(float r, float g, float b, float a);

    void attachUniformBufferObject(GLuint uniformBufferObjectID, const std::string& blockName, GLuint bindingPoint);
    void resizeTextures(const int newWidth, const int newHeight);

private:

    Canvas drawCanvas_;
    Shader vertexShader_;
    Shader fragmentShader_;
    ShaderProgram rasterizationPipeline_;

    Bloom bloomEffect_;

    UIState* uiState_;
    UniversalShaderSettings& ui_uss_;	//just shortening the name as a temp solution
    
    Texture texTrail_;		        //Texture Unit 0
    Texture texTrailNonDiffused_;	//Texture Unit 1
    Texture newTexParticles_;		//Texture Unit 2
    Texture oldTexParticles_;		//Texture Unit 3
    Texture texCollisions_;         //Texture Unit 4
};

#endif // RENDERER_H