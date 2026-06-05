#version 460

layout(local_size_x = 8, local_size_y = 8) in;
layout(rgba32f, binding = 0) uniform image2D texTrail;
layout(rgba32f, binding = 1) uniform image2D texTrailNonDiffused;
layout(rgba32f, binding = 2) uniform image2D newTexParticles;
layout(rgba32f, binding = 3) uniform image2D oldTexParticles;
layout(rgba32f, binding = 4) uniform image2D texCollision;

uniform int textureWidth;
uniform int textureHeight;

uniform int clearTexture;

uniform int copyTrailBuffer;
uniform int copyParticleBuffer;

// Hash function www.cs.ubc.ca/~rbridson/docs/schechter-sca08-turbulence.pdf
uint hash(uint state)
{
    state ^= 2747636419u;
    state *= 2654435769u;
    state ^= state >> 16;
    state *= 2654435769u;
    state ^= state >> 16;
    state *= 2654435769u;
    return state;
}

float scaleToRange01(uint state)
{
    return state/4294967295.0f;
}

void main() {

    ivec2 posPx = ivec2(gl_GlobalInvocationID.xy);

    if(copyTrailBuffer == 1) {
        imageStore(texTrailNonDiffused, posPx, imageLoad(texTrail, posPx));
    }

    if(copyParticleBuffer == 1) {
        imageStore(oldTexParticles, posPx, imageLoad(newTexParticles, posPx));
    }

    switch (clearTexture) {
    case 0:
        imageStore(texTrail, posPx, vec4(0.0f, 0.0f, 0.0f, 0.0f));
        break;
    case 1:
        imageStore(texTrailNonDiffused, posPx, vec4(0.0f, 0.0f, 0.0f, 0.0f));
        break;
    case 2:
        imageStore(newTexParticles, posPx, vec4(0.0f, 0.0f, 0.0f, 0.0f));
        break;
    case 3:
        imageStore(oldTexParticles, posPx, vec4(0.0f, 0.0f, 0.0f, 0.0f));
        break;
    case 4:
        imageStore(texCollision, posPx, vec4(0.0f, 0.0f, 0.0f, 0.0f));
        break;
    }

    imageStore(texCollision, posPx, vec4(0.0f, 0.0f, 0.0f, 0.0f));
}