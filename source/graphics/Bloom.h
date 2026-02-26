#ifndef BLOOM_H
#define BLOOM_H

#include "Shader.h"
#include "FrameBuffer.h"
#include "Texture.h"
#include "ShaderProgram.h"
#include "../canvas.h"


class Bloom {
public:
    Bloom(unsigned int textureWidth, unsigned int textureHeight, GLuint vertexShaderID);

    void applyBloom(GLuint texTrailID, Canvas& drawCanvas, const UIState& uiState);
    void bindBloomTextures(GLuint texTrail, GLuint texTrailNonDiffused, GLuint newTexParticles, GLuint oldTexParticles, GLuint texCollisions);
    void resizeBloomTextures(unsigned int textureWidth, unsigned int textureHeight);

private:

	const int BLOOM_MIPS_ = 5;

    Texture thresholdTexture_;
    FrameBuffer thresholdFramebuffer_;

	std::vector<Texture> bloomTextures_;
    std::vector<FrameBuffer> bloomFramebuffers_;
    std::vector<Texture> upsampleTextures_;
    std::vector<FrameBuffer> upsampleFramebuffers_;


    Shader BloomTresholdShader_;
	ShaderProgram BloomTresholdProgram_;

    Shader BloomDownsampleHShader_;
	ShaderProgram BloomDownsampleHProgram_;

    Shader BloomDownsampleVShader_;
	ShaderProgram BloomDownsampleVProgram_;

    Shader BloomUpsampleShader_;
	ShaderProgram BloomUpsampleProgram_;
};

#endif // BLOOM_H