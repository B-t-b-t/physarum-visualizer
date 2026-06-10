#ifndef RENDERER_H
#define RENDERER_H

#include "../canvas.h"
#include "shader.h"
#include "shader_program.h"

class Renderer {
public:

    Renderer();

    void draw();
    void clear(float r, float g, float b, float a);

    void attachUniformBufferObject(GLuint uniformBufferObjectID, const std::string& blockName, GLuint bindingPoint);

    Canvas& getCanvas() { return drawCanvas_; }
    Shader& getVertexShader() { return vertexShader_; }

private:

    Canvas drawCanvas_;
    Shader vertexShader_;
    Shader fragmentShader_;
    ShaderProgram rasterizationPipeline_;

    UIState* uiState_;
    UniversalShaderSettings& ui_uss_;	//just shortening the name as a temp solution
};

#endif // RENDERER_H