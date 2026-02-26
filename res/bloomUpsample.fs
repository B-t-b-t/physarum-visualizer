out vec4 FragColor;
in vec2 TexCoord;
layout (binding = 0) uniform sampler2D srcTexture;
layout (binding = 1) uniform sampler2D smallerMipTexture;

void main() {
    vec3 low = texture(smallerMipTexture, TexCoord).rgb;
    vec3 high = texture(srcTexture, TexCoord).rgb;
    FragColor = vec4(low + high, 1.0);
}
