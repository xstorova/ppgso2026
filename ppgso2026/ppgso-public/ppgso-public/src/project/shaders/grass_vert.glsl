#version 330 core

// Instanced vertex attributes
layout(location = 0) in vec3 Position;
layout(location = 1) in vec2 TexCoord;
layout(location = 2) in vec3 Normal;
layout(location = 3) in mat4 InstanceModelMatrix; // occupies locations 3, 4, 5, 6

uniform mat4 ViewMatrix;
uniform mat4 ProjectionMatrix;
uniform mat4 LightSpaceMatrix;
uniform float Time;

out vec3 FragPos;
out vec2 texCoord;
out vec3 Normal_out;
out vec4 FragPosLightSpace;

void main() {
    mat4 model = InstanceModelMatrix;

    // Jemny a prirodzeny vankok (iba na samotne spicky cepeli vysoko nad zemou)
    vec3 localPos = Position;
    if (localPos.y > 4.0) {
        float heightFactor = clamp((localPos.y - 4.0) / 8.0, 0.0, 1.0);
        float sway = sin(Time * 1.5 + model[3][0] * 0.3 + model[3][2] * 0.3) * 0.08 * (heightFactor * heightFactor);
        localPos.x += sway;
        localPos.z += sway * 0.4;
    }

    vec4 worldPos = model * vec4(localPos, 1.0);
    FragPos = worldPos.xyz;
    texCoord = TexCoord;

    // Normal transform
    mat3 normalMatrix = mat3(model);
    Normal_out = normalize(normalMatrix * Normal);

    FragPosLightSpace = LightSpaceMatrix * worldPos;
    gl_Position = ProjectionMatrix * ViewMatrix * worldPos;
}
