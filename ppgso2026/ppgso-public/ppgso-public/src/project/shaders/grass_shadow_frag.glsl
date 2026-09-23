#version 330 core
in vec2 texCoord;
uniform sampler2D Texture;

void main() {
    vec4 texColor = texture(Texture, texCoord);
    if (texColor.a < 0.25) {
        discard;
    }
}
