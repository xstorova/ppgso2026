#version 330 core

layout(location = 0) in vec3 Position;
layout(location = 1) in vec2 TexCoord;
layout(location = 2) in vec3 Normal;

uniform mat4 ProjectionMatrix;
uniform mat4 ViewMatrix;
uniform mat4 ModelMatrix;

// =================================================================================
// [13b] BOD 7 (4b): Shadow-maps - Matica transformacie do priestoru svetla
// =================================================================================
uniform mat4 lightSpaceMatrix;

out vec3 FragPos;
out vec2 texCoord;
out vec3 Normal_out;
out vec4 FragPosLightSpace;

void main() {
  vec4 worldPos = ModelMatrix * vec4(Position, 1.0);
  FragPos = vec3(worldPos);

  texCoord = TexCoord;

  // Normal matrix pre spravnu transformaciu normal pri nerovnomernom skalovani
  mat3 normalMatrix = transpose(inverse(mat3(ModelMatrix)));
  Normal_out = normalize(normalMatrix * Normal);

  // [13b] BOD 7 (4b): Pozicia fragmentu v priestore svetla pre vzorkovanie tienovej mapy
  FragPosLightSpace = lightSpaceMatrix * worldPos;

  gl_Position = ProjectionMatrix * ViewMatrix * worldPos;
}
