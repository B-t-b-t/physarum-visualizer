#version 460

in vec4 color;
in vec2 texCoord;

out vec4 fragColor;

uniform sampler2D fontAtlas;

void main() {
    vec4 glyphColor = vec4(texture(fontAtlas, texCoord).r) * color;

    // stb_truetype creates a font atlas with dark outlines.
    // Color these outlines in the same color as the glyphs to make them invisible.
    // Otherwise the dark outline would influence the physarum behaviour.
    if(glyphColor.a > 0.0f && (glyphColor.r != color.r || glyphColor.g != color.g || glyphColor.b != color.b)) {
        glyphColor = vec4(color.r, color.g, color.b, glyphColor.a);
    }

    fragColor = glyphColor;
}
