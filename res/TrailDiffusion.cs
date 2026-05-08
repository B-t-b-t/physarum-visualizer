layout(local_size_x = 8, local_size_y = 8) in;
layout(rgba32f, binding = 0) uniform image2D texTrail;
layout(rgba32f, binding = 1) uniform image2D texTrailNonDiffused;
layout(r32ui, binding = 2) uniform uimage2D newTexParticles;
layout(r32ui, binding = 3) uniform uimage2D oldTexParticles;
layout(rgba32f, binding = 4) uniform image2D texCollisions;
layout(binding = 5) uniform sampler2D texTrailMask;  //sampler2D because of read only

layout(std140, binding = 0) uniform UniversalShaderSettings {
    int textureWidth;
    int textureHeight;
    int windowWidth;
    int windowHeight;

    int renderParticles;
    int renderCollisions;
    int collisionDetection;
    int timeTicks;

    vec4 mouseInputs; // x, y, leftClick, rightClick
};

layout(std140, binding = 2) uniform TrailDiffusionSettings {
    float diffusionWeight;
    float decay;
    int useTrailMask;
};

// Helper function for boundary wrapping
ivec2 wrapCoordinates_i(ivec2 pos, int textureWidth, int textureHeight) {
    return ivec2(mod(pos.x + textureWidth, textureWidth), mod(pos.y + textureHeight, textureHeight));
}

void main() {

  // get index in global work group i.e x,y position
  ivec2 posPx = ivec2(gl_GlobalInvocationID.xy);
  vec3 thisPx = imageLoad(texTrailNonDiffused, posPx).rgb;

  vec3 sum = vec3(0.0f);

  for (int offsetX = -1; offsetX <= 1; offsetX ++) {
		for (int offsetY = -1; offsetY <= 1; offsetY ++) {
      ivec2 samplePos = wrapCoordinates_i(ivec2(posPx.x + offsetX, posPx.y + offsetY), textureWidth, textureHeight);
			sum += imageLoad(texTrailNonDiffused, samplePos).rgb;
		}
	}

  sum /= 9.0f;
  vec3 outPx = thisPx * (1.0f - diffusionWeight) + sum * diffusionWeight;
  
  // Decay just RGB, not alpha
  outPx = outPx * (1.0f - decay);
  outPx = clamp(outPx, 0.0f, 1.0f);

  //for debugging texTrailMask: if the mask value is above a certain threshold, add a bright spot to the trail texture (this should make it easier to see if the mask is correctly aligned and applied)
  //vec2 uv = vec2(posPx) / vec2(textureWidth, textureHeight);
  //vec3 maskValue = textureLod(texTrailMask, uv, 0.0f).rgb;

  //outPx = 0.5f * maskValue.rgb;

  //Trails speichern in Textur
  imageStore(texTrail, posPx, vec4(outPx, 1.0f));
  memoryBarrier();  //barrier necessary to ensure that all writes to thisPx from neighbouring texels have been completed
  imageStore(texTrailNonDiffused, posPx, vec4(outPx, 1.0f));

  if(collisionDetection == 1 || renderParticles == 1) {
    imageStore(oldTexParticles, posPx, imageLoad(newTexParticles, posPx));
    memoryBarrier();
    imageStore(newTexParticles, posPx, uvec4(0u));
  }

  if(collisionDetection == 1 && renderCollisions == 1) {
    imageStore(texCollisions, posPx, vec4(0.0f));
  }
}