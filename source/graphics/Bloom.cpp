#include "Bloom.h"

Bloom::Bloom(unsigned int textureWidth, unsigned int textureHeight, GLuint vertexShaderID)
   :thresholdTexture_((int) textureWidth, (int) textureHeight, 15, false),
    BloomTresholdShader_("./res/bloomThreshold.fs", ShaderType::FRAGMENT_SHADER),
	BloomTresholdProgram_("BloomTresholdProgram"), 
    BloomDownsampleHShader_("./res/bloomDownsampleH.fs", ShaderType::FRAGMENT_SHADER),
	BloomDownsampleHProgram_("BloomDownsampleHProgram"),
	BloomDownsampleVShader_("./res/bloomDownsampleV.fs", ShaderType::FRAGMENT_SHADER),
	BloomDownsampleVProgram_("BloomDownsampleVProgram"),
    BloomUpsampleShader_("./res/bloomUpsample.fs", ShaderType::FRAGMENT_SHADER),
	BloomUpsampleProgram_("BloomUpsampleProgram")
{
    // Texture Unit 15 (use no mipmaps for bloom textures!!!)

    //------------------------------------------------------
	thresholdFramebuffer_.attachTexture(GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, thresholdTexture_.getID(), 0);

	bloomTextures_.reserve(static_cast<size_t>(BLOOM_MIPS_));		//Reserve necessary because of missing Default and copy constructor in Texture
	bloomFramebuffers_.reserve(static_cast<size_t>(BLOOM_MIPS_));
	upsampleTextures_.reserve(static_cast<size_t>(BLOOM_MIPS_));
	upsampleFramebuffers_.reserve(static_cast<size_t>(BLOOM_MIPS_));

	for(size_t i = 0; i < static_cast<size_t>(BLOOM_MIPS_); ++i) {
		int w = (int) textureWidth >> (i + 1);
		int h = (int) textureHeight >> (i + 1);
		bloomTextures_.emplace_back(w, h, 5 + 2 * i,false); // Texture Units 5,7,9,11,13
		bloomFramebuffers_.emplace_back();			//emplace_back avoids copy constructor
		bloomFramebuffers_[i].attachTexture(GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, bloomTextures_[i].getID(), 0);

		upsampleTextures_.emplace_back(w, h, 6 + 2 * i, false);			// Texture Units 6,8,10,12,14
		upsampleFramebuffers_.emplace_back();
		upsampleFramebuffers_[i].attachTexture(GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, upsampleTextures_[i].getID(), 0);
	}

    //------------------------------------------------------
	//initialize Bloom Shaders

	BloomTresholdProgram_.attachShader(vertexShaderID);
	BloomTresholdProgram_.attachShader(BloomTresholdShader_.getShaderID());
	BloomTresholdProgram_.link();
	BloomTresholdProgram_.getUniformsFromGLSL();

	BloomDownsampleHProgram_.attachShader(BloomDownsampleHShader_.getShaderID());
	BloomDownsampleHProgram_.attachShader(vertexShaderID);
	BloomDownsampleHProgram_.link();
	BloomDownsampleHProgram_.getUniformsFromGLSL();

	BloomDownsampleVProgram_.attachShader(BloomDownsampleVShader_.getShaderID());
	BloomDownsampleVProgram_.attachShader(vertexShaderID);
	BloomDownsampleVProgram_.link();
	BloomDownsampleVProgram_.getUniformsFromGLSL();

	BloomUpsampleProgram_.attachShader(BloomUpsampleShader_.getShaderID());
	BloomUpsampleProgram_.attachShader(vertexShaderID);
	BloomUpsampleProgram_.link();
	BloomUpsampleProgram_.getUniformsFromGLSL();
}

void Bloom::applyBloom(GLuint texTrailID, Canvas& drawCanvas, const UIState& uiState) {
    thresholdFramebuffer_.bind();
    glClear(GL_COLOR_BUFFER_BIT);
    glViewport(0, 0, (int)uiState.universalShaderSettings.textureWidth, (int)uiState.universalShaderSettings.textureHeight);
    BloomTresholdProgram_.use();
    
    // Bind trail texture to unit 0 for bloom threshold processing
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texTrailID);
    BloomTresholdProgram_.setUniform1f("bloomThreshold", uiState.fragmentShaderSettings.bloomThreshold);
    // Upload soft-knee parameter for bloom threshold
    BloomTresholdProgram_.setUniform1f("bloomKnee", uiState.fragmentShaderSettings.bloomKnee);
    BloomTresholdProgram_.setUniform1i("srcTexture", 0);

    drawCanvas.draw();

	//Downsample pass
    GLuint currentSrc = thresholdTexture_.getID();
    for (size_t i = 0; i < static_cast<size_t>(BLOOM_MIPS_); ++i) {
        int w = (int) uiState.universalShaderSettings.textureWidth >> (i + 1);
        int h = (int) uiState.universalShaderSettings.textureHeight >> (i + 1);

        // Horizontal blur
        glBindFramebuffer(GL_FRAMEBUFFER, bloomFramebuffers_[i].getID());
        glViewport(0, 0, w, h);
        glClear(GL_COLOR_BUFFER_BIT);
        BloomDownsampleHProgram_.use();
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, currentSrc);
        BloomDownsampleHProgram_.setUniform1i("srcTexture", 0);
        BloomDownsampleHProgram_.setUniform2f("texelSize", 1.0f / w, 0.0f);
        drawCanvas.draw();

        // Vertical blur
        glBindFramebuffer(GL_FRAMEBUFFER, upsampleFramebuffers_[i].getID());
        glViewport(0, 0, w, h);
        glClear(GL_COLOR_BUFFER_BIT);
        BloomDownsampleVProgram_.use();
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, bloomTextures_[i].getID());
        BloomDownsampleVProgram_.setUniform1i("srcTexture", 0);
        BloomDownsampleVProgram_.setUniform2f("texelSize", 0.0f, 1.0f / h);
        drawCanvas.draw();

        currentSrc = upsampleTextures_[i].getID();
    }

	//Upsample pass
	for (int i = static_cast<int>(BLOOM_MIPS_) - 2; i >= 0; --i) {
		int w = (int) uiState.universalShaderSettings.textureWidth >> (i + 1);
		int h = (int) uiState.universalShaderSettings.textureHeight >> (i + 1);
		size_t upsampleIndex = static_cast<size_t>(i);

		// Render into the upsample texture at this mip level,
		// combining the current mip's blurred result with the smaller mip's upsampled result
		glBindFramebuffer(GL_FRAMEBUFFER, upsampleFramebuffers_[upsampleIndex].getID());
		glViewport(0, 0, w, h);
		glClear(GL_COLOR_BUFFER_BIT);
		BloomUpsampleProgram_.use();

		// Bind the current mip's blurred (downsample) result
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, bloomTextures_[upsampleIndex].getID());
		BloomUpsampleProgram_.setUniform1i("srcTexture", 0);

		// Bind the smaller (lower-res) mip's upsampled result to blend with
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, upsampleTextures_[upsampleIndex + 1].getID());
		BloomUpsampleProgram_.setUniform1i("smallerMipTexture", 1);

		drawCanvas.draw();
	}

    // Unbind framebuffer to ensure no read/write conflicts
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void Bloom::bindBloomTextures(GLuint texTrail, GLuint texTrailNonDiffused, GLuint newTexParticles, GLuint oldTexParticles, GLuint texCollisions) {
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, texTrail);
	glActiveTexture(GL_TEXTURE0 + 1);
	glBindTexture(GL_TEXTURE_2D, texTrailNonDiffused);
	glActiveTexture(GL_TEXTURE0 + 2);
	glBindTexture(GL_TEXTURE_2D, newTexParticles);
	glActiveTexture(GL_TEXTURE0 + 3);
	glBindTexture(GL_TEXTURE_2D, oldTexParticles);
	glActiveTexture(GL_TEXTURE0 + 4);
	glBindTexture(GL_TEXTURE_2D, texCollisions);
	glActiveTexture(GL_TEXTURE0 + 5);
	glBindTexture(GL_TEXTURE_2D, bloomTextures_[0].getID());
	glActiveTexture(GL_TEXTURE0 + 6);
	glBindTexture(GL_TEXTURE_2D, upsampleTextures_[0].getID());
	glActiveTexture(GL_TEXTURE0 + 7);
	glBindTexture(GL_TEXTURE_2D, bloomTextures_[1].getID());
	glActiveTexture(GL_TEXTURE0 + 8);
	glBindTexture(GL_TEXTURE_2D, upsampleTextures_[1].getID());
	glActiveTexture(GL_TEXTURE0 + 9);
	glBindTexture(GL_TEXTURE_2D, bloomTextures_[2].getID());
	glActiveTexture(GL_TEXTURE0 + 10);
	glBindTexture(GL_TEXTURE_2D, upsampleTextures_[2].getID());
	glActiveTexture(GL_TEXTURE0 + 11);
	glBindTexture(GL_TEXTURE_2D, bloomTextures_[3].getID());
	glActiveTexture(GL_TEXTURE0 + 12);
	glBindTexture(GL_TEXTURE_2D, upsampleTextures_[3].getID());
	glActiveTexture(GL_TEXTURE0 + 13);
	glBindTexture(GL_TEXTURE_2D, bloomTextures_[4].getID());
	glActiveTexture(GL_TEXTURE0 + 14);
	glBindTexture(GL_TEXTURE_2D, upsampleTextures_[4].getID());
	glActiveTexture(GL_TEXTURE0 + 15);
	glBindTexture(GL_TEXTURE_2D, thresholdTexture_.getID());
}

void Bloom::resizeBloomTextures(unsigned int textureWidth, unsigned int textureHeight) {
	thresholdTexture_.resizeTexture((int)textureWidth, (int)textureHeight);

	for(size_t i = 0; i < static_cast<size_t>(BLOOM_MIPS_); ++i) {
		int mipWidth = std::max(1, static_cast<int>(textureWidth) >> static_cast<int>(i + 1));
		int mipHeight = std::max(1, static_cast<int>(textureHeight) >> static_cast<int>(i + 1));
		bloomTextures_[i].resizeTexture(mipWidth, mipHeight);
		upsampleTextures_[i].resizeTexture(mipWidth, mipHeight);
	}
}
