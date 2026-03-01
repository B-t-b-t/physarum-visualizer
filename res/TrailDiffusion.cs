layout(local_size_x = 8, local_size_y = 8) in;
layout(rgba32f, binding = 0) uniform image2D texTrail;
layout(rgba32f, binding = 1) uniform image2D texTrailNonDiffused;
layout(rgba32f, binding = 2) uniform image2D newTexParticles;
layout(rgba32f, binding = 3) uniform image2D oldTexParticles;
layout(rgba32f, binding = 4) uniform image2D texCollisions;

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


  //Trails speichern in Textur
  imageStore(texTrail, posPx, vec4(outPx, 1.0f));
  memoryBarrier();  //barrier necessary to ensure that all writes to thisPx from neighbouring texels have been completed
  imageStore(texTrailNonDiffused, posPx, vec4(outPx, 1.0f));

  if(collisionDetection == 1 || renderParticles == 1) {
    imageStore(oldTexParticles, posPx, imageLoad(newTexParticles, posPx));
    memoryBarrier();
    imageStore(newTexParticles, posPx, vec4(0.0f));
  }

  if(collisionDetection == 1 && renderCollisions == 1) {
    imageStore(texCollisions, posPx, vec4(0.0f));
  }
}