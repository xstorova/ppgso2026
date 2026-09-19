#ifndef PROJECT_SCENE_H
#define PROJECT_SCENE_H

#include <list>
#include <map>
#include <memory>
#include <vector>

#include <ppgso/ppgso.h>

#include "camera.h"
#include "light.h"
#include "object.h"
#include "shadow_map.h"

class Scene {
public:
  Scene();

  void update(float dt);
  void render(float width, float height);
  void handleKey(int key, int action);

  void uploadLighting(const ppgso::Shader &shader) const;

  std::map<int, int> keyboard;
  std::list<std::unique_ptr<Object>> objects;
  Camera camera;

  // Svetelné zdroje
  DirLight dirLight;
  std::vector<PointLight> pointLights;
  SpotLight spotLight;

  // Nastavenia osvetľovacieho modelu
  bool useBlinnPhong{true};
  bool useHDR{true};
  float gamma{2.2f};
  bool normalMapEnabled{true};

  // =================================================================================
  // [13b] BOD 7 (4b): Shadow-maps - Správa tieňovej mapy a matica priestoru svetla
  // =================================================================================
  std::unique_ptr<ShadowMap> shadowMap;
  std::unique_ptr<ppgso::Shader> depthShader;
  bool shadowsEnabled{true};
  mutable glm::mat4 lightSpaceMatrix{1.0f};
};

#endif // PROJECT_SCENE_H
