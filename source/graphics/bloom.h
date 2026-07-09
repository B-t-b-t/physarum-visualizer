#ifndef BLOOM_H
#define BLOOM_H

#include "../canvas.h"
#include "framebuffer.h"
#include "shader.h"
#include "shader_program.h"
#include "texture.h"


class Bloom {
public:
    Bloom() = default;
    Bloom(int textureWidth, int textureHeight, Shader* vertexShader);

    void applyBloom(GLuint texTrailID, Canvas* drawCanvas, const ApplicationState* appState);
    void bindBloomTextures(GLuint texTrail, GLuint texTrailNonDiffused, GLuint newTexParticles, GLuint oldTexParticles, GLuint texCollisions);
    void resizeBloomTextures(int textureWidth, int textureHeight);

private:

	static constexpr int BLOOM_MIPS_ = 5;

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