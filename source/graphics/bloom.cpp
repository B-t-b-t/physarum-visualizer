#include "bloom.h"

Bloom::Bloom(int textureWidth, int textureHeight, Shader* vertexShader)
   :thresholdTexture_(TextureProperties{.width = textureWidth, .height = textureHeight, .texelFormat = TexelFormat::RGBA32F, .textureUnit = 15, .wrapX = TextureWrap::CLAMP_TO_EDGE, .wrapY = TextureWrap::CLAMP_TO_EDGE, .generateMipmaps = false, .minFilter = TextureMinFilter::LINEAR}),
    BloomTresholdShader_("./res/bloomThreshold.fs", ShaderType::FRAGMENT_SHADER),
	BloomTresholdProgram_("BloomTresholdProgram", {vertexShader, &BloomTresholdShader_}), 
    BloomDownsampleHShader_("./res/bloomDownsampleH.fs", ShaderType::FRAGMENT_SHADER),
	BloomDownsampleHProgram_("BloomDownsampleHProgram", {vertexShader, &BloomDownsampleHShader_}),
	BloomDownsampleVShader_("./res/bloomDownsampleV.fs", ShaderType::FRAGMENT_SHADER),
	BloomDownsampleVProgram_("BloomDownsampleVProgram", {vertexShader, &BloomDownsampleVShader_}),
    BloomUpsampleShader_("./res/bloomUpsample.fs", ShaderType::FRAGMENT_SHADER),
	BloomUpsampleProgram_("BloomUpsampleProgram", {vertexShader, &BloomUpsampleShader_})
{
    // Texture Unit 15 (use no mipmaps for bloom textures!!!)

    //------------------------------------------------------
	thresholdFramebuffer_.attachTexture(GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, thresholdTexture_.getID(), 0);

	bloomTextures_.reserve(static_cast<size_t>(BLOOM_MIPS_));		//Reserve necessary because of missing Default and copy constructor in Texture
	bloomFramebuffers_.reserve(static_cast<size_t>(BLOOM_MIPS_));
	upsampleTextures_.reserve(static_cast<size_t>(BLOOM_MIPS_));
	upsampleFramebuffers_.reserve(static_cast<size_t>(BLOOM_MIPS_));
	TextureProperties properties;
	properties.generateMipmaps = false;
	properties.minFilter = TextureMinFilter::LINEAR;

	for(size_t level = 0; level < static_cast<size_t>(BLOOM_MIPS_); ++level) {
		int w = textureWidth >> (level + 1);
		int h = textureHeight >> (level + 1);
		properties.width = w;
		properties.height = h;
		//properties.texelFormat = TexelFormat::RGBA16F;	//TODO: using 16bit is 25% faster, but check for HDR precission requirements
		properties.textureUnit = 5 + 2 * level; // Texture Units 5,7,9,11,13 for bloom textures
		bloomTextures_.emplace_back(properties);
		bloomFramebuffers_.emplace_back();			//emplace_back avoids copy constructor
		bloomFramebuffers_[level].attachTexture(GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, bloomTextures_[level].getID(), 0);

		properties.textureUnit = 6 + 2 * level; // Texture Units 6,8,10,12,14 for upsample textures
		upsampleTextures_.emplace_back(properties);
		upsampleFramebuffers_.emplace_back();
		upsampleFramebuffers_[level].attachTexture(GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, upsampleTextures_[level].getID(), 0);
	}
}

void Bloom::applyBloom(GLuint texTrailID, Canvas* drawCanvas, const ApplicationState* appState) {
    thresholdFramebuffer_.bind();
    glViewport(0, 0, (int)appState->universalShaderSettings.textureWidth, (int)appState->universalShaderSettings.textureHeight);
    BloomTresholdProgram_.use();
    
    // Bind trail texture to unit 0 for bloom threshold processing
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texTrailID);
    BloomTresholdProgram_.setUniform1f("bloomThreshold", appState->fragmentShaderSettings.bloomThreshold);
    BloomTresholdProgram_.setUniform1f("bloomKnee", appState->fragmentShaderSettings.bloomKnee);
    BloomTresholdProgram_.setUniform1i("srcTexture", 0);

    drawCanvas->draw();

	//Downsample pass
    GLuint currentSrc = thresholdTexture_.getID();
    for (size_t level = 0; level < static_cast<size_t>(BLOOM_MIPS_); ++level) {
        int w = (int) appState->universalShaderSettings.textureWidth >> (level + 1);
        int h = (int) appState->universalShaderSettings.textureHeight >> (level + 1);

        // Horizontal blur
        glBindFramebuffer(GL_FRAMEBUFFER, bloomFramebuffers_[level].getID());
        glViewport(0, 0, w, h);
        BloomDownsampleHProgram_.use();
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, currentSrc);
        BloomDownsampleHProgram_.setUniform1i("srcTexture", 0);
        BloomDownsampleHProgram_.setUniform2f("texelSize", 1.0f / w, 0.0f);
        drawCanvas->draw();

        // Vertical blur
        glBindFramebuffer(GL_FRAMEBUFFER, upsampleFramebuffers_[level].getID());
        glViewport(0, 0, w, h);
        BloomDownsampleVProgram_.use();
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, bloomTextures_[level].getID());
        BloomDownsampleVProgram_.setUniform1i("srcTexture", 0);
        BloomDownsampleVProgram_.setUniform2f("texelSize", 0.0f, 1.0f / h);
        drawCanvas->draw();

        currentSrc = upsampleTextures_[level].getID();
    }

	//Upsample pass
	for (int level = static_cast<int>(BLOOM_MIPS_) - 2; level >= 0; --level) {
		int w = (int) appState->universalShaderSettings.textureWidth >> (level + 1);
		int h = (int) appState->universalShaderSettings.textureHeight >> (level + 1);
		size_t upsampleIndex = static_cast<size_t>(level);

		//bind Bloom textures to draw into
		upsampleFramebuffers_[upsampleIndex].attachTexture(GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, bloomTextures_[upsampleIndex].getID(), 0);

		// Render into the upsample texture at this mip level,
		// combining the current mip's blurred result with the smaller mip's upsampled result
		glBindFramebuffer(GL_FRAMEBUFFER, upsampleFramebuffers_[upsampleIndex].getID());
		glViewport(0, 0, w, h);
		BloomUpsampleProgram_.use();

		// Bind the current mip's blurred (downsample) result
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, upsampleTextures_[upsampleIndex].getID());
		BloomUpsampleProgram_.setUniform1i("srcTexture", 0);

		// Bind the smaller (lower-res) mip's upsampled result to blend with
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, upsampleTextures_[upsampleIndex + 1].getID());
		BloomUpsampleProgram_.setUniform1i("smallerMipTexture", 1);

		drawCanvas->draw();

		//bind Bloom textures to draw into
		upsampleFramebuffers_[upsampleIndex].attachTexture(GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, upsampleTextures_[upsampleIndex].getID(), 0);
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

void Bloom::resizeBloomTextures(int textureWidth, int textureHeight) {
	thresholdTexture_.resizeTexture(textureWidth, textureHeight);

	for(size_t level = 0; level < static_cast<size_t>(BLOOM_MIPS_); ++level) {
		int mipWidth = std::max(1, textureWidth >> (level + 1));
		int mipHeight = std::max(1, textureHeight >> (level + 1));
		bloomTextures_[level].resizeTexture(mipWidth, mipHeight);
		upsampleTextures_[level].resizeTexture(mipWidth, mipHeight);
	}
}
