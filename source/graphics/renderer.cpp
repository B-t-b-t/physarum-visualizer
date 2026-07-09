#include "renderer.h"

Renderer::Renderer(UniformBufferManager* uboManager, ApplicationState* uiState)
 :  drawCanvas_(Canvas()),
    vertexShader_{Shader("./res/vertex.vs", ShaderType::VERTEX_SHADER)},
    fragmentShader_{Shader("./res/fragment.fs", ShaderType::FRAGMENT_SHADER)},
	rasterizationPipeline_{ShaderProgram("RasterizationPipeline", {&vertexShader_, &fragmentShader_})},
    appState_{uiState},
    ui_uss_{appState_->universalShaderSettings},
	texTrail_{(int)ui_uss_.textureWidth, (int)ui_uss_.textureHeight, Texture::TextureType::RGBA_FLOAT, 0},		//Texture Unit 0
	texTrailNonDiffused_{(int)ui_uss_.textureWidth, (int)ui_uss_.textureHeight, Texture::TextureType::RGBA_FLOAT, 1},	//Texture Unit 1
	newTexParticles_{(int)ui_uss_.textureWidth, (int)ui_uss_.textureHeight, Texture::TextureType::R_UINT, 2},		//Texture Unit 2
	oldTexParticles_{(int)ui_uss_.textureWidth, (int)ui_uss_.textureHeight, Texture::TextureType::R_UINT, 3},		//Texture Unit 3
	texCollisions_{(int)ui_uss_.textureWidth, (int)ui_uss_.textureHeight, Texture::TextureType::RGBA_FLOAT, 4}		//Texture Unit 4
{
    uboManager->attachUBOs({rasterizationPipeline_.getProgramID()});

    // Initialize Bloom Effect
	bloomEffect_ = Bloom(ui_uss_.textureWidth, ui_uss_.textureHeight, &vertexShader_);
}

Renderer::Renderer(Renderer&& rhs) noexcept
    : drawCanvas_(std::move(rhs.drawCanvas_)),
      vertexShader_(std::move(rhs.vertexShader_)),
      fragmentShader_(std::move(rhs.fragmentShader_)),
      rasterizationPipeline_(std::move(rhs.rasterizationPipeline_)),
      bloomEffect_(std::move(rhs.bloomEffect_)),
      appState_(rhs.appState_),
      ui_uss_(rhs.ui_uss_),
      texTrail_(std::move(rhs.texTrail_)),
      texTrailNonDiffused_(std::move(rhs.texTrailNonDiffused_)),
      newTexParticles_(std::move(rhs.newTexParticles_)),
      oldTexParticles_(std::move(rhs.oldTexParticles_)),
      texCollisions_(std::move(rhs.texCollisions_))
{
}

Renderer& Renderer::operator=(Renderer&& rhs) noexcept {
    if (this != &rhs) {
        drawCanvas_ = std::move(rhs.drawCanvas_);
        vertexShader_ = std::move(rhs.vertexShader_);
        fragmentShader_ = std::move(rhs.fragmentShader_);
        rasterizationPipeline_ = std::move(rhs.rasterizationPipeline_);
        bloomEffect_ = std::move(rhs.bloomEffect_);
        appState_ = rhs.appState_;
        ui_uss_ = rhs.ui_uss_;
        texTrail_ = std::move(rhs.texTrail_);
        texTrailNonDiffused_ = std::move(rhs.texTrailNonDiffused_);
        newTexParticles_ = std::move(rhs.newTexParticles_);
        oldTexParticles_ = std::move(rhs.oldTexParticles_);
        texCollisions_ = std::move(rhs.texCollisions_);
    }
    return *this;
}

void Renderer::draw() {
	// Bind main textures for fragment shader (these should always be bound)
	bloomEffect_.bindBloomTextures(texTrail_.getID(), texTrailNonDiffused_.getID(), newTexParticles_.getID(), oldTexParticles_.getID(), texCollisions_.getID());

    //------------------------------------------------------
    // Bloom Post-Processing
    if(appState_->fragmentShaderSettings.bloomEnabled) {
        bloomEffect_.applyBloom(texTrail_.getID(), &drawCanvas_, appState_);
    }

    //------------------------------------------------------
    // Display Clearing
    clear(appState_->clearColor.x, appState_->clearColor.y, appState_->clearColor.z, appState_->clearColor.w);

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

void Renderer::resizeTextures(const int newWidth, const int newHeight) {
    texTrail_.resizeTexture(newWidth, newHeight);
	texTrailNonDiffused_.resizeTexture(newWidth, newHeight);
	newTexParticles_.resizeTexture(newWidth, newHeight);
	oldTexParticles_.resizeTexture(newWidth, newHeight);
	texCollisions_.resizeTexture(newWidth, newHeight);

	bloomEffect_.resizeBloomTextures(newWidth, newHeight);

    //glViewport(0, 0, newWidth, newHeight);
}
