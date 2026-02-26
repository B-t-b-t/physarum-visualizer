out vec4 FragColor;
in vec2 TexCoord;
layout (binding = 0) uniform sampler2D srcTexture;
uniform vec2 texelSize;
uniform int stage;

void main() {
    float weights[5] = float[5](0.06136, 0.24477, 0.38774, 0.24477, 0.06136);
    vec3 color = vec3(0.0);
    for(int i = -2; i <= 2; ++i) {
        color += texture(srcTexture, TexCoord + vec2(0, i) * texelSize).rgb * weights[i+2];
    }
    FragColor = vec4(color, 1.0);

/*     if(texelSize.y >= (1/448.0)) {
        FragColor = vec4(1.0, 0.0, 0.0, 1.0);
    } */
    //FragColor = vec4(1.0, 0.0, 0.0, 1.0);
}
