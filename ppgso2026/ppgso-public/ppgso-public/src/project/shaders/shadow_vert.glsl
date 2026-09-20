#version 330 core

// =================================================================================
// [13b] BOD 7 (4b): Shadow-maps - Vertex shader pre hlbkovy prechod (depth pass)
// Transformuje vrcholy do priestoru svetla (light space)
// =================================================================================

layout(location = 0) in vec3 Position;

uniform mat4 lightSpaceMatrix;
uniform mat4 ModelMatrix;

void main() {
  gl_Position = lightSpaceMatrix * ModelMatrix * vec4(Position, 1.0);
}
