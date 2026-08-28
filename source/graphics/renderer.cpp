#include "renderer.h"

Renderer::Renderer(UniformBufferManager* uboManager, ApplicationState* appState)
 :  drawCanvas_(Canvas()),
    vertexShader_{Shader("./res/vertex.vs", ShaderType::VERTEX_SHADER)},
    fragmentShader_{Shader("./res/fragment.fs", ShaderType::FRAGMENT_SHADER)},
	rasterizationPipeline_{ShaderProgram("RasterizationPipeline", {&vertexShader_, &fragmentShader_})},
    appState_{appState},
    app_uss_{appState_->universalShaderSettings},
    texTrail_{TextureProperties{.width = app_uss_.textureWidth, .height = app_uss_.textureHeight, .textureUnit = 0, .imageUnit = 0}},    //Texture Unit 0
	texTrailNonDiffused_{TextureProperties{.width = app_uss_.textureWidth, .height = app_uss_.textureHeight, .textureUnit = 1, .imageUnit = 1}},	//Texture Unit 1
	newTexParticles_{TextureProperties{.width = app_uss_.textureWidth, .height = app_uss_.textureHeight, .texelFormat = TexelFormat::R_UINT, .textureUnit = 2, .imageUnit = 2}},		//Texture Unit 2
	oldTexParticles_{TextureProperties{.width = app_uss_.textureWidth, .height = app_uss_.textureHeight, .texelFormat = TexelFormat::R_UINT, .textureUnit = 3, .imageUnit = 3}},		//Texture Unit 3
	texCollisions_{TextureProperties{.width = app_uss_.textureWidth, .height = app_uss_.textureHeight, .textureUnit = 4, .imageUnit = 4}}		//Texture Unit 4
{
    uboManager->attachUBOs({rasterizationPipeline_.getProgramID()});

    // Initialize Bloom Effect
	bloomEffect_ = Bloom(app_uss_.textureWidth, app_uss_.textureHeight, &vertexShader_);
}

Renderer::Renderer(Renderer&& rhs) noexcept
    : drawCanvas_(std::move(rhs.drawCanvas_)),
      vertexShader_(std::move(rhs.vertexShader_)),
      fragmentShader_(std::move(rhs.fragmentShader_)),
      rasterizationPipeline_(std::move(rhs.rasterizationPipeline_)),
      bloomEffect_(std::move(rhs.bloomEffect_)),
      appState_(rhs.appState_),
      app_uss_(rhs.app_uss_),
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
        app_uss_ = rhs.app_uss_;
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
    glViewport(0, 0, (int) app_uss_.windowWidth, (int) app_uss_.windowHeight);
    
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

void Renderer::onNotify(const Event event) {
    switch(event) {
        case Event::NEW_CANVAS:
            if(appState_->newTextureWidth != app_uss_.textureWidth || appState_->newTextureHeight != app_uss_.textureHeight) {
                resizeTextures(appState_->newTextureWidth, appState_->newTextureHeight);
            }
            break;
        default:
            break;
    }
}