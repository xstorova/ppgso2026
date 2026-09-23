#version 330 core
layout(location = 0) in vec3 Position;
layout(location = 1) in vec2 TexCoord;
layout(location = 3) in mat4 InstanceModelMatrix;

uniform mat4 LightSpaceMatrix;
out vec2 texCoord;

void main() {
    texCoord = TexCoord;
    gl_Position = LightSpaceMatrix * InstanceModelMatrix * vec4(Position, 1.0);
}
