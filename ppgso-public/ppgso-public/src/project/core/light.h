#ifndef PROJECT_LIGHT_H
#define PROJECT_LIGHT_H

#include <glm/glm.hpp>

// =================================================================================
// [13b] BOD 4 (2b): Správny Phongov model - Tri zložky farby materiálu:
// ambient, diffuse, specular (+ lesk shininess)
// =================================================================================
struct Material {
  glm::vec3 ambient{0.2f, 0.2f, 0.2f};
  glm::vec3 diffuse{0.8f, 0.8f, 0.8f};
  glm::vec3 specular{0.5f, 0.5f, 0.5f};
  float shininess{32.0f};
};

// =================================================================================
// [13b] BOD 1 (3b): Všetky typy zdrojov svetla:
// Typ 1: SMEROVÉ SVETLO (Directional Light) - smerové parametre a 3 zložky svetla
// =================================================================================
struct DirLight {
  bool enabled{false};
  glm::vec3 direction{-0.2f, -1.0f, -0.3f};
  glm::vec3 ambient{0.05f, 0.05f, 0.08f};
  glm::vec3 diffuse{0.3f, 0.3f, 0.4f};
  glm::vec3 specular{0.2f, 0.2f, 0.3f};
};

// =================================================================================
// [13b] BOD 1 (3b): Všetky typy zdrojov svetla:
// Typ 2: BODOVÉ SVETLO (Point Light) - pozícia, 3 zložky svetla a kvadratický útlm
// [13b] BOD 4: Správne tlmenie svetla na základe hĺbky/vzdialenosti
// =================================================================================
struct PointLight {
  bool enabled{true};
  glm::vec3 position{0.0f, 1.0f, 0.0f}; // Pozícia v strede miestnosti pod stromom
  glm::vec3 ambient{0.08f, 0.06f, 0.04f};
  glm::vec3 diffuse{1.4f, 1.1f, 0.8f}; // Teplé svetlo pod stromom
  glm::vec3 specular{1.0f, 0.95f, 0.85f};
  float constant{1.0f};
  float linear{0.09f};
  float quadratic{0.032f};
};

// =================================================================================
// [13b] BOD 1 (3b): Všetky typy zdrojov svetla:
// Typ 3: REFLEKTOR (Spotlight) - pozícia, smer, kužeľové uhly (cutOff) a tlmenie
// =================================================================================
struct SpotLight {
  bool enabled{false};
  glm::vec3 position{0.0f, 5.0f, 0.0f};
  glm::vec3 direction{0.0f, -1.0f, 0.0f};
  glm::vec3 ambient{0.0f, 0.0f, 0.0f};
  glm::vec3 diffuse{1.0f, 1.0f, 1.0f};
  glm::vec3 specular{1.0f, 1.0f, 1.0f};
  float constant{1.0f};
  float linear{0.09f};
  float quadratic{0.032f};
  float cutOff{glm::cos(glm::radians(12.5f))};
  float outerCutOff{glm::cos(glm::radians(17.5f))};
};

#endif // PROJECT_LIGHT_H
