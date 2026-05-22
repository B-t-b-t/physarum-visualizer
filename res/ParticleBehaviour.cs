layout(local_size_x = 8, local_size_y = 1) in;
layout(rgba32f, binding = 0) uniform image2D texTrail;
layout(rgba32f, binding = 1) uniform image2D texTrailNonDiffused;
layout(r32ui, binding = 2) uniform uimage2D newTexParticles;
layout(r32ui, binding = 3) uniform uimage2D oldTexParticles;
layout(rgba32f, binding = 4) uniform image2D texCollisions;
layout(binding = 5) uniform sampler2D texTrailMask;  //sampler2D because of read only
/*layout(rgba32f, binding = 1) uniform image2D canvas;*/
/*uniform sampler2D trails;*/
/*uniform sampler2D canvas;*/

struct Particle {
    vec2 position;
    float angle;
    float speciesID;
};

struct SensedTrail {
    float left;
    float front;
    float right;
};

layout(std140, binding = 0) buffer ParticleBuffer{
    Particle particleArray[];
};

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

layout(std140, binding = 1) uniform SlimeSettings {
    float v;
    float depositionStrength;
    int rotationAngle;
    int angle;

    int sensorDistance;
    uint densityLimit;
    int useMask;
    float velocityBassReaction;

    int reactToAudio;
    int angleBassReaction;

    vec4 slimeColor0;
    vec4 slimeColor1;
    vec4 slimeColor2;

    vec4 particleColor0;
    vec4 particleColor1;
    vec4 particleColor2;
    vec4 collisionColor;
};

layout(std140, binding = 4) uniform ParameterSettings {
    float p1;
    float p2;
    float p3;
    float p4;

    float p5;
    float p6;
    float p7;
    float p8;
    
    float p9;
    float p10;
    float p11;
    float p12;

    int enableParameters;
};

//function declarations
uint hash(uint state);
float scaleToRange01(uint state);
vec2 wrapCoordinates_f(vec2 pos, int textureWidth, int textureHeight);
ivec2 wrapCoordinates_i(ivec2 pos, int textureWidth, int textureHeight);
vec4 when_eq(vec4 x, vec4 y);
vec4 when_gt(vec4 x, vec4 y);
Particle moveParticle(Particle particle, float beatVel, float ds, vec3 speciesColor, vec3 particleColor);
SensedTrail sensingTrail(Particle particle, float beatSensorDistance, float da, vec3 speciesColor, vec3 speciesMask);
Particle rotateToNewDirection(Particle particle, SensedTrail sensedTrail, float ds);


void main() {

    float PI = 3.1415926535f;
    float da = 2.0f * PI * ((angle + angleBassReaction) / 360.0f);
    float ds = 2.0f * PI * ((rotationAngle + angleBassReaction) / 360.0f);

    float beatVel = v + reactToAudio * velocityBassReaction * 0.3f;

    int beatSensorDistance = sensorDistance;// + reactToAudio * velocityBassReaction;

    uint invocationID = gl_GlobalInvocationID.x;

    Particle particle = particleArray[invocationID];


    vec3 speciesColor = vec3(0.0f, 0.0f, 0.0f);
    vec3 particleColor = vec3(0.0f, 0.0f, 0.0f);
    vec3 speciesMask = vec3(0.0f, 0.0f, 0.0f);

    switch(int(particle.speciesID)) {
        case 1:
            speciesColor = slimeColor0.rgb;
            particleColor = particleColor0.rgb;
            speciesMask = vec3(1.0f, -1.0f, -1.0f);
            break;
        case 2:
            speciesColor = slimeColor1.rgb;
            particleColor = particleColor1.rgb;
            speciesMask = vec3(-1.0f, 1.0f, -1.0f);
            break;
        case 3:
            speciesColor = slimeColor2.rgb;
            particleColor = particleColor2.rgb;
            speciesMask = vec3(-1.0f, -1.0f, 1.0f);
            break;
    }

    // int speciesIDInt = int(speciesID);
    // speciesColor = slimeColor0 * step(0.9, speciesID) * step(-1.1 ,-speciesID) + slimeColor1 * step(1.9, speciesID) * step(-2.1 ,-speciesID) + slimeColor2 * step(2.9, speciesID) * step(-3.1 ,-speciesID);
    // particleColor = particleColor0 * step(0.9, speciesID) * step(-1.1 ,-speciesID) + particleColor1 * step(1.9, speciesID) * step(-2.1 ,-speciesID) + particleColor2 * step(2.9, speciesID) * step(-3.1 ,-speciesID);
    // speciesMask = vec4(1.0f, -1.0f, -1.0f, 0.0f) * step(0.9, speciesID) * step(-1.1 ,-speciesID) + vec4(-1.0f, 1.0f, -1.0f, 0.0f) * step(1.9, speciesID) * step(-2.1 ,-speciesID) + vec4(-1.0f, -1.0f, 1.0f, 0.0f) * step(2.9, speciesID) * step(-3.1 ,-speciesID);

    // int speciesIDInt = int(speciesID);
    // vec4 eq = when_eq(vec4(1.0f, 2.0f, 3.0f, 4.0f), vec4(speciesID));

    // speciesColor = slimeColor0 * eq.x + slimeColor1 * eq.y + slimeColor2 * eq.z;
    // particleColor = particleColor0 * eq.x + particleColor1 * eq.y + particleColor2 * eq.z;
    // speciesMask = vec4(1.0f, -1.0f, -1.0f, 0.0f) * eq.x + vec4(-1.0f, 1.0f, -1.0f, 0.0f) * eq.y + vec4(-1.0f, -1.0f, 1.0f, 0.0f) * eq.z;



    //---------------------------------Particle Movement---------------------------------

    vec2 mousePos = vec2(mouseInputs.x, mouseInputs.y);
    
    mousePos.y = windowHeight - mousePos.y;
    vec2 mouseVector = particle.position - vec2(mousePos.x * (textureWidth / float(windowWidth)), mousePos.y * (textureHeight / float(windowHeight)));

    if(length(mouseVector) < 100.0f) {
        if(mouseInputs.z > 0.0f) {
            beatVel = 100.0f * beatVel;
        }
        if(mouseInputs.w > 0.0f) {
            beatVel = 0.0 * beatVel;
        }
    }

    if(enableParameters == 1) {
        float trailDensity = length(imageLoad(texTrail, ivec2(particle.position)));
        beatSensorDistance = int(p1 + p2 * pow(trailDensity , p3));
        ds =  p4 + p5 * pow(trailDensity , p6);
        da =  p7 + p8 * pow(trailDensity , p9);
        beatVel = p10 + p11 * pow(trailDensity , p12);
    }

    Particle newParticle = moveParticle(particle, beatVel, ds, speciesColor, particleColor);

    //---------------------------------Particle Sensing---------------------------------
    //sense TrailMap
    SensedTrail sensedTrail = sensingTrail(newParticle, beatSensorDistance, da, speciesColor, speciesMask);

    //new direction
    newParticle = rotateToNewDirection(newParticle, sensedTrail, ds);
    newParticle.angle = mod(newParticle.angle, 2*PI);


    particleArray[invocationID] = newParticle;
}

// Hash function www.cs.ubc.ca/~rbridson/docs/schechter-sca08-turbulence.pdf
uint hash(uint state) {
    state ^= 2747636419u;
    state *= 2654435769u;
    state ^= state >> 16;
    state *= 2654435769u;
    state ^= state >> 16;
    state *= 2654435769u;
    return state;
}

float scaleToRange01(uint state) {
    return state/4294967295.0f;
}

// Helper function for boundary wrapping
vec2 wrapCoordinates_f(vec2 pos, int textureWidth, int textureHeight) {
    return vec2(mod(pos.x + textureWidth, textureWidth), mod(pos.y + textureHeight, textureHeight));
}

// Helper function for boundary wrapping
ivec2 wrapCoordinates_i(ivec2 pos, int textureWidth, int textureHeight) {
    return ivec2(mod(pos.x + textureWidth, textureWidth), mod(pos.y + textureHeight, textureHeight));
}

vec4 when_eq(vec4 x, vec4 y) {
  return 1.0 - abs(sign(x - y));
}

vec4 when_gt(vec4 x, vec4 y) {
  return max(sign(x - y), 0.0);
}

Particle moveParticle(Particle particle, float beatVel, float ds, vec3 speciesColor, vec3 particleColor) {

    vec2 newParticleCoords = vec2(particle.position.x + beatVel * cos(particle.angle), particle.position.y + beatVel * sin(particle.angle));
    //move particle to the other side of the screen if it goes out of bounds
    newParticleCoords = wrapCoordinates_f(newParticleCoords, textureWidth, textureHeight);

    Particle updatedParticle = particle;
    int canMove = 1;

    if(renderParticles == 1 || collisionDetection == 1) {
        if(collisionDetection == 1) {
            //read old particle count at the target position from previous frame
            uint oldCount = imageLoad(oldTexParticles, ivec2(newParticleCoords)).r;
            if(oldCount >= densityLimit) {
                //next pixel was already occupied last frame, therefore stay at old particle position
                imageAtomicAdd(newTexParticles, ivec2(particle.position), 1u);
                canMove = 0;
            } else {
                //next pixel is free to claim
                imageAtomicAdd(newTexParticles, ivec2(newParticleCoords), 1u);
            }
        } else {
            // renderParticles only, just count without collision enforcement
            imageAtomicAdd(newTexParticles, ivec2(newParticleCoords), 1u);
        }
    }

    if(canMove == 1) {
        updatedParticle.position = newParticleCoords;
        imageStore(texTrailNonDiffused, ivec2(newParticleCoords), vec4(speciesColor * depositionStrength, 1.0f));

    } else {
        float randomTurn = scaleToRange01(hash(uint(particle.position.x * particle.position.y + timeTicks)));
        particle.angle = particle.angle + ds * 2 * (randomTurn - 0.5f);
        updatedParticle.angle = particle.angle;

        if(collisionDetection == 1) {
            imageStore(texCollisions, ivec2(particle.position), collisionColor);
        }
    }

    return updatedParticle;
}

SensedTrail sensingTrail(Particle particle, float beatSensorDistance, float da, vec3 speciesColor, vec3 speciesMask) {
    SensedTrail sensedTrail = SensedTrail(0.0f, 0.0f, 0.0f);
    vec4 sensedPixelValue;

    //sense left Pixel
    ivec2 sensePos = ivec2(particle.position.x + beatSensorDistance * cos(particle.angle + da), particle.position.y + beatSensorDistance * sin(particle.angle + da));

    sensePos = wrapCoordinates_i(sensePos, textureWidth, textureHeight);

    sensedPixelValue = imageLoad(texTrail, sensePos);

    if(useMask == 1) {
        sensedTrail.left = dot(speciesMask, sensedPixelValue.rgb);
    } else {
        sensedTrail.left = dot(speciesColor.rgb, sensedPixelValue.rgb);
    }

    vec2 trailMaskPos = vec2((trailMaskScale * (sensePos / vec2(textureWidth, textureHeight) - 0.5f)) + 0.5f);    //transalte with 0.5 to scale from center
    sensedPixelValue = trailMaskInfluence * textureLod(texTrailMask, trailMaskPos, 0.0f);
    sensedTrail.left += dot(speciesColor.rgb, sensedPixelValue.rgb);

    //sense front Pixel
    sensePos = ivec2(particle.position.x + beatSensorDistance * cos(particle.angle), particle.position.y + beatSensorDistance * sin(particle.angle));

    sensePos = wrapCoordinates_i(sensePos, textureWidth, textureHeight);

    sensedPixelValue = imageLoad(texTrail, sensePos);

    if(useMask == 1) {
        sensedTrail.front = dot(speciesMask, sensedPixelValue.rgb);
    } else {
        sensedTrail.front = dot(speciesColor.rgb, sensedPixelValue.rgb);
    }

    trailMaskPos = vec2((trailMaskScale * (sensePos / vec2(textureWidth, textureHeight) - 0.5f)) + 0.5f);
    sensedPixelValue = trailMaskInfluence * textureLod(texTrailMask, trailMaskPos, 0.0f);
    sensedTrail.front += dot(speciesColor.rgb, sensedPixelValue.rgb);

    //sense right Pixel
    sensePos = ivec2(particle.position.x + beatSensorDistance * cos(particle.angle - da), particle.position.y + beatSensorDistance * sin(particle.angle - da));

    sensePos = wrapCoordinates_i(sensePos, textureWidth, textureHeight);

    sensedPixelValue = imageLoad(texTrail, sensePos);

    if(useMask == 1) {
        sensedTrail.right = dot(speciesMask, sensedPixelValue.rgb);
    } else {
        sensedTrail.right = dot(speciesColor.rgb, sensedPixelValue.rgb);
    }

    trailMaskPos = vec2((trailMaskScale * (sensePos / vec2(textureWidth, textureHeight) - 0.5f)) + 0.5f);
    sensedPixelValue = trailMaskInfluence * textureLod(texTrailMask, trailMaskPos, 0.0f);
    sensedTrail.right += dot(speciesColor.rgb, sensedPixelValue.rgb);

    return sensedTrail;
}

Particle rotateToNewDirection(Particle particle, SensedTrail sensedTrail, float ds) {

    //front strongest
    if((sensedTrail.front > sensedTrail.left) && (sensedTrail.front > sensedTrail.right)) {
        return particle;
    }

    //front weakest or all directions equal
    if((sensedTrail.front <= sensedTrail.left) && (sensedTrail.front <= sensedTrail.right)) {
        float randomTurn = scaleToRange01(hash(uint(particle.position.x * particle.position.y + timeTicks)));
        
        particle.angle = particle.angle + ds * 2 * (randomTurn - 0.5f);  //turn random
        return particle;
    }

    if(sensedTrail.left < sensedTrail.right) {  //right strongest
        particle.angle = particle.angle - ds;   //turn right
        return particle;
    }
    else if(sensedTrail.right < sensedTrail.left) { //left strongest
        particle.angle = particle.angle + ds;       //turn left
        return particle;
    } else {
        return particle;
    }
}