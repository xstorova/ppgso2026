#version 330 core

struct Material {
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
    float shininess;
};

struct DirLight {
    bool enabled;
    vec3 direction;
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

struct PointLight {
    bool enabled;
    vec3 position;
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
    float constant;
    float linear;
    float quadratic;
};

#define MAX_POINT_LIGHTS 4

uniform sampler2D Texture;
uniform sampler2D shadowMap;
uniform bool shadowsEnabled;

uniform vec3 CameraPosition;
uniform Material material;
uniform DirLight dirLight;
uniform PointLight pointLights[MAX_POINT_LIGHTS];
uniform int numPointLights;
uniform bool useHDR;
uniform float gamma;

in vec3 FragPos;
in vec2 texCoord;
in vec3 Normal_out;
in vec4 FragPosLightSpace;

out vec4 FragmentColor;

float ShadowCalculation(vec4 fragPosLightSpace, vec3 normal, vec3 lightDir) {
    if (!shadowsEnabled) return 0.0;
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    projCoords = projCoords * 0.5 + 0.5;
    if (projCoords.z > 1.0) return 0.0;

    float currentDepth = projCoords.z;
    float bias = max(0.005 * (1.0 - dot(normal, lightDir)), 0.001);
    float shadow = 0.0;
    vec2 texelSize = 1.0 / textureSize(shadowMap, 0);
    for (int x = -1; x <= 1; ++x) {
        for (int y = -1; y <= 1; ++y) {
            float pcfDepth = texture(shadowMap, projCoords.xy + vec2(x, y) * texelSize).r;
            shadow += currentDepth - bias > pcfDepth ? 1.0 : 0.0;
        }
    }
    return shadow / 9.0;
}

void main() {
    vec4 texColor = texture(Texture, texCoord);
    // Alpha discard pre travnate cepele
    if (texColor.a < 0.25) {
        discard;
    }

    vec3 norm = normalize(Normal_out);
    if (!gl_FrontFacing) {
        norm = -norm;
    }

    vec3 viewDir = normalize(CameraPosition - FragPos);
    vec3 result = vec3(0.0);

    // Directional light (Slnko)
    if (dirLight.enabled) {
        vec3 lightDir = normalize(-dirLight.direction);
        float diff = max(dot(norm, lightDir), 0.35); // rozptylene svetlo cez tenke listy
        vec3 diffuse = dirLight.diffuse * (diff * material.diffuse);
        vec3 ambient = dirLight.ambient * material.ambient;

        float shadow = ShadowCalculation(FragPosLightSpace, norm, lightDir);
        result += ambient + (1.0 - shadow) * diffuse;
    } else {
        result += material.ambient;
    }

    // Bodove svetla
    for (int i = 0; i < numPointLights && i < MAX_POINT_LIGHTS; ++i) {
        if (!pointLights[i].enabled) continue;
        vec3 lightDir = normalize(pointLights[i].position - FragPos);
        float diff = max(dot(norm, lightDir), 0.0);
        float distance = length(pointLights[i].position - FragPos);
        float attenuation = 1.0 / (pointLights[i].constant + pointLights[i].linear * distance + pointLights[i].quadratic * (distance * distance));

        vec3 ambient = pointLights[i].ambient * material.ambient * attenuation;
        vec3 diffuse = pointLights[i].diffuse * (diff * material.diffuse) * attenuation;
        result += ambient + diffuse;
    }

    vec3 finalColor = result * texColor.rgb;

    if (useHDR) {
        finalColor = vec3(1.0) - exp(-finalColor * 1.0);
    }
    finalColor = pow(finalColor, vec3(1.0 / (gamma > 0.0 ? gamma : 2.2)));

    FragmentColor = vec4(finalColor, 1.0);
}
