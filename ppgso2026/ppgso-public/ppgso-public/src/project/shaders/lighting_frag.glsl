#version 330 core

// =================================================================================
// [13b] BOD 4 (2b): Spravny Phongov osvetlovaci model
// - Pouzite tri zlozky farby materialu: ambient, diffuse, specular + shininess
// =================================================================================
struct Material {
  vec3 ambient;
  vec3 diffuse;
  vec3 specular;
  float shininess;
};

// =================================================================================
// [13b] BOD 1 (3b): Vsetky typy zdrojov svetla:
// 1. SMEROVE SVETLO (Directional Light) - smer + 3 zlozky svetla
// =================================================================================
struct DirLight {
  bool enabled;
  vec3 direction;
  vec3 ambient;
  vec3 diffuse;
  vec3 specular;
};

// =================================================================================
// [13b] BOD 1 (3b): Vsetky typy zdrojov svetla:
// 2. BODOVE SVETLO (Point Light) - pozicia + 3 zlozky svetla + parametre tlmenia
// =================================================================================
struct PointLight {
  bool enabled;
  vec3 position;
  vec3 ambient;
  vec3 diffuse;
  vec3 specular;
  // [13b] BOD 4: Tlmenie svetla na zaklade hlbky/vzdialenosti (attenuation)
  float constant;
  float linear;
  float quadratic;
};

// =================================================================================
// [13b] BOD 1 (3b): Vsetky typy zdrojov svetla:
// 3. REFLEKTOR (Spotlight) - pozicia, smer, kuzelove uhly (cutOff) + tlmenie
// =================================================================================
struct SpotLight {
  bool enabled;
  vec3 position;
  vec3 direction;
  vec3 ambient;
  vec3 diffuse;
  vec3 specular;
  float constant;
  float linear;
  float quadratic;
  float cutOff;
  float outerCutOff;
};

#define MAX_POINT_LIGHTS 4

uniform sampler2D Texture;
uniform vec2 TextureOffset;
uniform vec2 TextureScale;
uniform float Transparency;

// =================================================================================
// [1b] Objekt s vyuzitim hrbolatej textury (Normal Mapping)
// =================================================================================
uniform sampler2D normalMap;
uniform bool useNormalMap;
uniform bool normalMapGlobalEnabled;

// =================================================================================
// [13b] BOD 7 (4b): Shadow-maps - Vzorkovac hlbkovej textury a prepinac tienov
// =================================================================================
uniform sampler2D shadowMap;
uniform bool shadowsEnabled;

uniform vec3 CameraPosition;
uniform Material material;

uniform DirLight dirLight;
uniform PointLight pointLights[MAX_POINT_LIGHTS];
uniform int numPointLights;
uniform SpotLight spotLight;

// Prepinace osvetlovacich modelov
uniform bool useBlinnPhong;
uniform bool useHDR;
uniform float gamma;

in vec3 FragPos;
in vec2 texCoord;
in vec3 Normal_out;
in vec4 FragPosLightSpace;

out vec4 FragmentColor;

// =================================================================================
// [1b] Vypocet tangentneho priestoru (TBN matice) pomocou cotangent frame (screen-space derivacie)
// =================================================================================
mat3 cotangent_frame(vec3 N, vec3 p, vec2 uv) {
  // Smerove derivacie polohy v priestore a texturovych suradnic na povrchu trojuholnika
  vec3 dp1 = dFdx(p);
  vec3 dp2 = dFdy(p);
  vec2 duv1 = dFdx(uv);
  vec2 duv2 = dFdy(uv);

  // Vypocet kolmic na derivacie
  vec3 dp2perp = cross(dp2, N);
  vec3 dp1perp = cross(N, dp1);

  // Tangent a Bitangent
  vec3 T = dp2perp * duv1.x + dp1perp * duv2.x;
  vec3 B = dp2perp * duv1.y + dp1perp * duv2.y;

  // Normalizacia ramca TBN
  float invmax = inversesqrt(max(dot(T, T), dot(B, B)) + 1e-6);
  return mat3(T * invmax, B * invmax, N);
}

// =================================================================================
// [13b] BOD 7 (4b): Vypocet tiena pomocou Shadow-maps s Percentage-Closer Filtering (PCF)
// =================================================================================
float ShadowCalculation(vec4 fragPosLightSpace, vec3 normal, vec3 lightDir) {
  if (!shadowsEnabled) return 0.0;

  // Perspektivne delenie
  vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
  // Prevod z NDC (-1..1) do rozsahu textury (0..1)
  projCoords = projCoords * 0.5 + 0.5;

  // Za far plane svetla sa tien neuplatnuje
  if (projCoords.z > 1.0) return 0.0;

  // Dynamicky bias podla sklonu povrchu voci smeru svetla
  float bias = max(0.004 * (1.0 - dot(normal, lightDir)), 0.001);

  // 3x3 PCF filter pre vyhladene, realisticke okraje tiena
  float shadow = 0.0;
  vec2 texelSize = 1.0 / textureSize(shadowMap, 0);
  for (int x = -1; x <= 1; ++x) {
    for (int y = -1; y <= 1; ++y) {
      float pcfDepth = texture(shadowMap, projCoords.xy + vec2(x, y) * texelSize).r;
      shadow += (projCoords.z - bias > pcfDepth) ? 1.0 : 0.0;
    }
  }
  shadow /= 9.0;

  return shadow;
}

// =================================================================================
// VYPOCET SMEROVEHO SVETLA (Directional Light)
// =================================================================================
vec3 CalcDirLight(DirLight light, vec3 normal, vec3 viewDir, vec3 texColor) {
  if (!light.enabled) return vec3(0.0);

  vec3 lightDir = normalize(-light.direction);

  // Difuzna zlozka
  float diff = max(dot(normal, lightDir), 0.0);

  // [13b] BOD 5 (1b): Blinn-Phong modifikovany model vs. klasicky Phong
  float spec = 0.0;
  if (useBlinnPhong) {
    vec3 halfwayDir = normalize(lightDir + viewDir);
    spec = pow(max(dot(normal, halfwayDir), 0.0), material.shininess);
  } else {
    vec3 reflectDir = reflect(-lightDir, normal);
    spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
  }

  // [13b] BOD 4: Spravna kombinacia troch zloziek materialu a svetla
  vec3 ambient = light.ambient * (material.ambient * texColor);
  vec3 diffuse = light.diffuse * (diff * (material.diffuse * texColor));
  vec3 specular = light.specular * (spec * material.specular);

  return ambient + diffuse + specular;
}

// =================================================================================
// VYPOCET BODOVEHO SVETLA (Point Light)
// =================================================================================
vec3 CalcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir, vec3 texColor, float shadow) {
  if (!light.enabled) return vec3(0.0);

  vec3 lightDir = normalize(light.position - fragPos);

  // Difuzna zlozka
  float diff = max(dot(normal, lightDir), 0.0);

  // [13b] BOD 5 (1b): Blinn-Phong modifikovany model
  float spec = 0.0;
  if (useBlinnPhong) {
    vec3 halfwayDir = normalize(lightDir + viewDir);
    spec = pow(max(dot(normal, halfwayDir), 0.0), material.shininess);
  } else {
    vec3 reflectDir = reflect(-lightDir, normal);
    spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
  }

  // [13b] BOD 4: Spravne tlmenie svetla na zaklade hlbky/vzdialenosti (attenuation)
  float distance = length(light.position - fragPos);
  float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * (distance * distance));

  vec3 ambient = light.ambient * (material.ambient * texColor);
  vec3 diffuse = light.diffuse * (diff * (material.diffuse * texColor));
  vec3 specular = light.specular * (spec * material.specular);

  // [13b] BOD 7 (4b): Shadow-maps - ambient zostava zachovany, diffuse a specular su tlmene tienom
  return (ambient + (1.0 - shadow) * (diffuse + specular)) * attenuation;
}

// =================================================================================
// VYPOCET REFLEKTORA (Spotlight)
// =================================================================================
vec3 CalcSpotLight(SpotLight light, vec3 normal, vec3 fragPos, vec3 viewDir, vec3 texColor, float shadow) {
  if (!light.enabled) return vec3(0.0);

  vec3 lightDir = normalize(light.position - fragPos);

  // Difuzna zlozka
  float diff = max(dot(normal, lightDir), 0.0);

  // [13b] BOD 5 (1b): Blinn-Phong
  float spec = 0.0;
  if (useBlinnPhong) {
    vec3 halfwayDir = normalize(lightDir + viewDir);
    spec = pow(max(dot(normal, halfwayDir), 0.0), material.shininess);
  } else {
    vec3 reflectDir = reflect(-lightDir, normal);
    spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
  }

  // Tlmenie vzdialenostou
  float distance = length(light.position - fragPos);
  float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * (distance * distance));

  // Kuzel reflektora s jemnym okrajom
  float theta = dot(lightDir, normalize(-light.direction));
  float epsilon = light.cutOff - light.outerCutOff;
  float intensity = clamp((theta - light.outerCutOff) / epsilon, 0.0, 1.0);

  vec3 ambient = light.ambient * (material.ambient * texColor);
  vec3 diffuse = light.diffuse * (diff * (material.diffuse * texColor));
  vec3 specular = light.specular * (spec * material.specular);

  return (ambient + (1.0 - shadow) * (diffuse * intensity + specular * intensity)) * attenuation;
}

void main() {
  vec3 normal = normalize(Normal_out);
  vec3 viewDir = normalize(CameraPosition - FragPos);

  // Vypocet texturovych suradnic s moznostou dlazdicovania (tiling) cez TextureScale
  vec2 scale = (TextureScale.x != 0.0 && TextureScale.y != 0.0) ? TextureScale : vec2(1.0);
  vec2 uv = (vec2(texCoord.x, 1.0 - texCoord.y) * scale) + TextureOffset;

  // =================================================================================
  // [1b] Objekt s vyuzitim hrbolatej textury (Normal Mapping)
  // - Vzorkovanie normalovej mapy z rozsahu [0, 1] do tangentneho priestoru [-1, 1]
  // - Transformacia normaly z tangentneho do svetoveho priestoru pomocou matice TBN
  // =================================================================================
  if (useNormalMap && normalMapGlobalEnabled) {
    vec3 mapNormal = texture(normalMap, uv).rgb * 2.0 - 1.0;
    mat3 TBN = cotangent_frame(normal, FragPos, uv);
    normal = normalize(TBN * mapNormal);
  }

  vec4 tex = texture(Texture, uv);
  vec3 texColor = tex.rgb;

  // [13b] BOD 7 (4b): Vypocet faktoru tiena zo shadow-mapy pre hlavne svetlo v miestnosti
  vec3 mainLightDir = (numPointLights > 0) ? normalize(pointLights[0].position - FragPos) : vec3(0.0, 1.0, 0.0);
  float shadow = ShadowCalculation(FragPosLightSpace, normal, mainLightDir);

  vec3 result = vec3(0.0);

  // Smerove svetlo (mesacne svetlo)
  result += CalcDirLight(dirLight, normal, viewDir, texColor);

  // [13b] BOD 3 (1b): Spravne kombinovat difuzne svetlo z viacerych zdrojov svetla s difuznymi materialmi
  for (int i = 0; i < numPointLights; ++i) {
    // Prve svetlo (pod stromom) vrha tien cez shadow mapu
    float s = (i == 0) ? shadow : 0.0;
    result += CalcPointLight(pointLights[i], normal, FragPos, viewDir, texColor, s);
  }

  // Reflektor (spotlight)
  result += CalcSpotLight(spotLight, normal, FragPos, viewDir, texColor, 0.0);

  // =================================================================================
  // [13b] BOD 6 (1b): HDR zobrazenie s pouzitim mapovania tonov a gama korekcie
  // =================================================================================
  if (useHDR) {
    vec3 mapped = result / (result + vec3(1.0));
    mapped = pow(mapped, vec3(1.0 / gamma));
    FragmentColor = vec4(mapped, Transparency * tex.a);
  } else {
    FragmentColor = vec4(result, Transparency * tex.a);
  }
}
