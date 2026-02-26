out vec4 FragColor;
in vec2 TexCoord;
uniform sampler2D srcTexture;
uniform float bloomThreshold;
uniform float bloomKnee;

void main() {
    vec3 color = texture(srcTexture, TexCoord).rgb;
    float brightness = max(max(color.r, color.g), color.b); //find brightest colour value
    
    // Soft knee: smoothly ramp contribution starting at threshold up to threshold + knee
    if (brightness > bloomThreshold) {
        float knee = max(0.0, bloomKnee);
        float contribution = (knee > 0.0) ? smoothstep(bloomThreshold, bloomThreshold + knee, brightness) : 1.0;
        FragColor = vec4(color * contribution, 1.0);
    } else {
        FragColor = vec4(0.0, 0.0, 0.0, 0.0);
    }
}
