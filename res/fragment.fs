out vec4 FragColor;  

/*layout(pixel_center_integer) in vec4 gl_FragCoord;*/

in vec2 TexCoord;
layout (binding = 0) uniform sampler2D texTrail;
layout (binding = 1) uniform sampler2D texTrailNonDiffused;
layout (binding = 2) uniform usampler2D newTexParticles;
layout (binding = 3) uniform usampler2D oldTexParticles;
layout (binding = 4) uniform sampler2D texCollisions;
layout (binding = 5) uniform sampler2D bloomTexture;
layout (binding = 6) uniform sampler2D upSample1;
layout (binding = 7) uniform sampler2D downSample2;
layout (binding = 8) uniform sampler2D upSample2;
layout (binding = 9) uniform sampler2D downSample3;
layout (binding = 10) uniform sampler2D upSample3;
layout (binding = 11) uniform sampler2D downSample4;
layout (binding = 12) uniform sampler2D upSample4;
layout (binding = 13) uniform sampler2D downSample5;
layout (binding = 14) uniform sampler2D upSample5;
layout (binding = 15) uniform sampler2D thresholdTexture;
layout (binding = 16) uniform sampler2D trailMask;

/*uniform sampler2D canvas;*/
//layout(rgba32f, binding = 0) uniform image2D trails;
//layout(rgba32f, binding = 1) uniform image2D texParticles;

layout(std140, binding = 0) uniform UniversalShaderSettings {
    int textureWidth;
    int textureHeight;
    int windowWidth;
    int windowHeight;

    int renderParticles;
    int renderCollisions;
    int collisionDetection;
	int timeTicks;

	float trailMaskInfluence;
	float trailMaskScale;
	vec2 trailMaskPosition;

    vec4 mouseInputs; // x, y, leftClick, rightClick
};

layout(std140, binding = 3) uniform FragmentShaderSettings {
	uniform float exposure;
	uniform int renderColorTraces;
	uniform int toneMappingMode; //0 = reinhard, 1 = exposure, 2 = ACES
	uniform int vignetteEffect;

	uniform float vignetteXDimension;
	uniform float vignetteYDimension;
	uniform float vignetteInnerRadius;
	uniform float vignetteSharpness;

	uniform int vignetteSelector;
	uniform int debugTextureMaskSelector;
	uniform int bloomEnabled;
	uniform int bloomBlendMode; //0 = additive, 1 = soft additive, 2 = screen

	uniform float bloomIntensity;
	uniform float bloomThreshold;
	uniform float bloomKnee;
	uniform float bloomBassReaction;

	uniform float brightnessMultiplier;
};


vec4 premult(vec4 color) {
    return vec4(color.rgb * color.a, color.a);
}

vec4 unpremult(vec4 color) {
    // Prevent division by zero
    if (color.a == 0.0)
        return vec4(0.0);
        
    return vec4(color.rgb / color.a, color.a);
}

void main() {
	const float gamma = 2.2f;
	vec4 sceneColour = texture(texTrail, TexCoord);
	vec4 debugOverlay = vec4(0.0f, 0.0f, 0.0f, 0.0f);
	vec2 pixelSize = 1.0 / vec2(windowWidth, windowHeight);

	// Per-mip weights for upsampled bloom textures, prevents bloom reaching too far from highest mips
 	float bloomWeights[5] = float[5](0.227027f, 0.1945946f, 0.1216216f, 0.054054f, 0.016216f);

	//render every trail at maximum brightness for debugging
	if(renderColorTraces == 1) {
		if(length(sceneColour.rgb) > 0.0f) {
			debugOverlay = vec4(1.0f, 1.0f, 1.0f, 1.0f);
		}
	}

	if(renderParticles == 1) {
        uint particleCount = texture(newTexParticles, TexCoord).r;
		float presence = min(float(particleCount) / 8.0f, 1.0f);
		vec4 particleColor = vec4(presence, presence, presence, 1.0f);
		if(particleCount == 0u) { particleColor = vec4(0.0f, 0.0f, 0.0f, 0.0f); }
		else if(particleCount == 1u) { particleColor = vec4(0.0f, 0.0f, 1.0f, 1.0f); }  // 1 = blue
		else if(particleCount == 2u) { particleColor = vec4(0.0f, 1.0f, 1.0f, 1.0f); }  // 2 = cyan
		else if(particleCount == 3u) { particleColor = vec4(0.0f, 1.0f, 0.0f, 1.0f); }  // 3 = green
		else if(particleCount <= 5u) { particleColor = vec4(1.0f, 1.0f, 0.0f, 1.0f); }  // 4-5 = yellow
		else if(particleCount <= 7u) { particleColor = vec4(1.0f, 0.5f, 0.0f, 1.0f); }  // 6-7 = orange
		else                        { particleColor = vec4(1.0f, 0.0f, 0.0f, 1.0f); }  // >8 = red
        debugOverlay = mix(debugOverlay, particleColor, particleColor.a);
	}

	if(renderCollisions == 1) {
		vec4 collisionColor = vec4(texture(texCollisions, TexCoord));
		debugOverlay = mix(debugOverlay, collisionColor, collisionColor.a);
	}

	vec3 hdr = sceneColour.rgb;

    // Modern bloom: add upsampled bloom texture
	if(bloomEnabled == 1 && bloomIntensity > 0.0) {
		vec3 bloomRGB = texture(upSample1, TexCoord).rgb;// * bloomWeights[0];
		//bloomRGB += texture(upSample2, TexCoord).rgb * bloomWeights[1];
		//bloomRGB += texture(upSample3, TexCoord).rgb * bloomWeights[2];
		//bloomRGB += texture(upSample4, TexCoord).rgb * bloomWeights[3];
		//bloomRGB += texture(upSample5, TexCoord).rgb * bloomWeights[4];

		if(bloomBlendMode == 0) {
        	// Method 1: Additive blending (most common)
        	hdr = hdr + bloomRGB * (bloomIntensity + bloomBassReaction);
		} else if(bloomBlendMode == 1) {
        	// Method 2: Screen blending (prevents over-brightening)
         	vec3 screenBlend = hdr + bloomRGB - (hdr * bloomRGB);
         	hdr = mix(hdr, screenBlend, (bloomIntensity + bloomBassReaction));
		} else if(bloomBlendMode == 2) {
        	// Method 3: Soft additive (Uncharted/COD style)
        	vec3 softAdd = hdr + bloomRGB * (bloomIntensity + bloomBassReaction) * (1.0 - hdr);
        	hdr = mix(hdr, softAdd, (bloomIntensity	 + bloomBassReaction));
		}
		hdr = max(hdr, vec3(0.0)); // clamp negative values to 0
	}

	// Brightness boost based on trail intensity (AFTER bloom, BEFORE tone mapping!)
    float trailIntensity = sceneColour.a;
    float brightnessBoost = 1.0 + (1.0 - trailIntensity) * brightnessMultiplier;
    hdr *= brightnessBoost;

	vec3 tone = vec3(0.0, 0.0, 0.0);

	if(toneMappingMode == 0) {
		//reinhard tone mapping
		tone = hdr / (hdr + vec3(1.0));
	} else if(toneMappingMode == 1) {
		//exposure tone mapping
		tone = vec3(1.0) - exp(-hdr * exposure);
	} else if(toneMappingMode == 2) {
		//ACES tone mapping
		const float a = 2.51f;
		const float b = 0.03f;
		const float c = 2.43f;
		const float d = 0.59f;
		const float e = 0.14f;
		tone = clamp((hdr * (a * hdr + b)) / (hdr * (c * hdr + d) + e), 0.0, 1.0);
	}

	//gamma correction
	vec3 finalRGB = pow(tone, vec3(1.0 / gamma));

	vec3 composited = mix(finalRGB, debugOverlay.rgb, debugOverlay.a);
	FragColor = vec4(composited, 1.0);

	//FragColor = vec4(finalRGB, 1.0) + debugOverlay;

	//vignette effect
	if(vignetteEffect == 1) {
		vec2 vectorToScreenCenter = TexCoord - vec2(0.5f, 0.5f);

		if(windowWidth > windowHeight) {
			vectorToScreenCenter.x *= windowWidth / windowHeight;
		} else {
			vectorToScreenCenter.y *= windowHeight / windowWidth;
		}

		vectorToScreenCenter.x *= vignetteXDimension;
		vectorToScreenCenter.y *= vignetteYDimension;
		
		if(vignetteSelector == 0) {
			FragColor = mix(FragColor, vec4(0.0, 0.0, 0.0, 1.0), pow(clamp(((2 * vignetteInnerRadius * length(vectorToScreenCenter) - vignetteInnerRadius) / 0.5f) + 1.0f, 0.0f, 1.0f), vignetteSharpness));
		} else {
			FragColor *= 1.0f - mix(vec4(0.0, 0.0, 0.0, 0.0), vec4(1.0, 1.0, 1.0, 1.0), pow(clamp(((2 * vignetteInnerRadius * length(vectorToScreenCenter) - vignetteInnerRadius) / 0.5f) + 1.0f, 0.0f, 1.0f), vignetteSharpness));
		}
	} 

	//debug view of textures
	if((gl_FragCoord.x / windowWidth <= 0.5) && (gl_FragCoord.y / windowHeight >= 0.5)) {
		if(debugTextureMaskSelector == 1) {FragColor = vec4(texture(texTrail, TexCoord));}
		if(debugTextureMaskSelector == 2) {FragColor = vec4(vec3(min(float(texture(newTexParticles, TexCoord).r) / 100.0f, 1.0f)), 1.0f);}
		if(debugTextureMaskSelector == 3) {FragColor = vec4(vec3(min(float(texture(oldTexParticles, TexCoord).r) / 100.0f, 1.0f)), 1.0f);}
		if(debugTextureMaskSelector == 4) {FragColor = vec4(texture(texCollisions, TexCoord));}
		if(debugTextureMaskSelector == 5) {FragColor = vec4(texture(bloomTexture, TexCoord));}
		if(debugTextureMaskSelector == 6) {FragColor = vec4(texture(upSample1, TexCoord));}
		if(debugTextureMaskSelector == 7) {FragColor = vec4(texture(downSample2, TexCoord));}
		if(debugTextureMaskSelector == 8) {FragColor = vec4(texture(upSample2, TexCoord));}
		if(debugTextureMaskSelector == 9) {FragColor = vec4(texture(downSample3, TexCoord));}
		if(debugTextureMaskSelector == 10) {FragColor = vec4(texture(upSample3, TexCoord));}
		if(debugTextureMaskSelector == 11) {FragColor = vec4(texture(downSample4, TexCoord));}
		if(debugTextureMaskSelector == 12) {FragColor = vec4(texture(upSample4, TexCoord));}
		if(debugTextureMaskSelector == 13) {FragColor = vec4(texture(downSample5, TexCoord));}
		if(debugTextureMaskSelector == 14) {FragColor = vec4(texture(upSample5, TexCoord));}
		//if(debugTextureMaskSelector == 15) {FragColor = vec4(texture(thresholdTexture, TexCoord));}
		if(debugTextureMaskSelector == 15) {FragColor = vec4(texture(trailMask, TexCoord));}
	}
}