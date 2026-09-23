#ifndef PROJECT_SOFA_H
#define PROJECT_SOFA_H

#include <ppgso/ppgso.h>
#include <memory>

#include "../core/object.h"
#include "../core/scene.h"

class Sofa : public Object {
public:
  Sofa(glm::vec3 position = glm::vec3(-4.8f, 0.0f, -0.5f),
       float yawAngleDeg = 155.0f,
       float scaleMultiplier = 0.024f);

  void render(const Scene &scene, float width, float height) override;
  void renderDepth(const ppgso::Shader &depthShader) override;

private:
  static std::unique_ptr<ppgso::Shader> shader;
  static std::unique_ptr<ppgso::Mesh> mesh;
  static std::shared_ptr<ppgso::Texture> diffuseTexture;
  static std::shared_ptr<ppgso::Texture> normalTexture;

  struct {
    glm::vec3 ambient;
    glm::vec3 diffuse;
    glm::vec3 specular;
    float shininess;
  } material;
};

#endif // PROJECT_SOFA_H
