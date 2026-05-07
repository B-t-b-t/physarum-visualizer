out vec4 FragColor;
in vec2 TexCoord;
uniform sampler2D srcTexture;
uniform float bloomThreshold;
uniform float bloomKnee;

void main() {
    vec3 color = texture(srcTexture, TexCoord).rgb;
    float brightness = max(color.r, max(color.g, color.b));//dot(color, vec3(0.3, 0.59, 0.11));    //0.2126, 0.7152, 0.0722 are the luminance coefficients for linear RGB
    /*float knee = bloomThreshold * bloomKnee;
    float soft = brightness - bloomThreshold + knee;
    soft = clamp(soft, 0.0, 2 * knee);
    soft = soft * soft / (4 * knee + 0.0001); // Avoid division by zero
    float contribution = max(0.0f, brightness - bloomThreshold);
    contribution /= max(brightness, 0.0001f); // Avoid division by zero
    FragColor = vec4(color * contribution, 1.0f);*/
    
    // Soft knee: smoothly ramp contribution starting at threshold up to threshold + knee
    if (brightness > bloomThreshold) {
        float knee = max(0.0, bloomKnee);
        float contribution = (knee > 0.0) ? smoothstep(bloomThreshold, bloomThreshold + knee, brightness) : 1.0;
        FragColor = vec4(color * (brightness - bloomThreshold) / brightness * contribution, 1.0);
    } else {
        FragColor = vec4(0.0, 0.0, 0.0, 1.0);
    }
}
