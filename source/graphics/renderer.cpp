#include "renderer.h"

Renderer::Renderer()
 :  drawCanvas_(Canvas()),
    vertexShader_{Shader("./res/vertex.vs", ShaderType::VERTEX_SHADER)},
    fragmentShader_{Shader("./res/fragment.fs", ShaderType::FRAGMENT_SHADER)},
	rasterizationPipeline_{ShaderProgram("RasterizationPipeline", {&vertexShader_, &fragmentShader_})},
    uiState_{UIState::getInstance()},
    ui_uss_{uiState_->universalShaderSettings}
{
}

void Renderer::draw() {
    //------------------------------------------------------
    // Display Clearing
    clear(uiState_->clearColor.x, uiState_->clearColor.y, uiState_->clearColor.z, uiState_->clearColor.w);

    //------------------------------------------------------
    //OpenGL Draw Call
    glBindFramebuffer(GL_FRAMEBUFFER, 0); // Default framebuffer
    glViewport(0, 0, (int) ui_uss_.windowWidth, (int) ui_uss_.windowHeight);
    rasterizationPipeline_.use();

    drawCanvas_.draw();
}

void Renderer::clear(float r, float g, float b, float a) {
    glClearColor(r, g, b, a);
    glClear(GL_COLOR_BUFFER_BIT);
}

void Renderer::attachUniformBufferObject(GLuint uniformBufferObjectID, const std::string& blockName, GLuint bindingPoint) {
    rasterizationPipeline_.attachUniformBufferObject(uniformBufferObjectID, blockName, bindingPoint);

}
