#ifndef PROJECT_LAMP_H
#define PROJECT_LAMP_H

#include <ppgso/ppgso.h>
#include <memory>

#include "../core/object.h"

class Scene;

class Lamp : public Object {
public:
  Lamp(glm::vec3 pos = glm::vec3(0.0f, 6.0f, 0.0f),
       float yawAngleDeg = 0.0f,
       float scaleMultiplier = 0.013f);

  glm::mat4 modelMatrix() const override;
  void render(const Scene &scene, float width, float height) override;
  void renderDepth(const ppgso::Shader &depthShader) override;

private:
  float yawRad{0.0f};
  float uniformScale{0.013f};
  glm::vec3 meshMountOffset{0.0f, -171.29f, 0.0f};

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

#endif // PROJECT_LAMP_H
