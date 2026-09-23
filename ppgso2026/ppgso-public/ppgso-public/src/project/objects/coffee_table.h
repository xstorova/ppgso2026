#ifndef PROJECT_COFFEE_TABLE_H
#define PROJECT_COFFEE_TABLE_H

#include <ppgso/ppgso.h>
#include <memory>

#include "../core/object.h"
#include "../core/scene.h"

class CoffeeTable : public Object {
public:
  CoffeeTable(glm::vec3 position = glm::vec3(0.0f, 0.0f, -4.5f),
              float yawAngleDeg = 0.0f,
              float scaleMultiplier = 0.003f);

  void render(const Scene &scene, float width, float height) override;
  void renderDepth(const ppgso::Shader &depthShader) override;

  glm::mat4 modelMatrix() const override;

private:
  float yawRad;
  float uniformScale;

  // Offset na vycentrovanie modelu (Z stred je daleko od originu)
  glm::vec3 meshCenterOffset;

  static std::unique_ptr<ppgso::Shader> shader;
  static std::unique_ptr<ppgso::Mesh> mesh;
  static std::unique_ptr<ppgso::Texture> texture;

  struct {
    glm::vec3 ambient;
    glm::vec3 diffuse;
    glm::vec3 specular;
    float shininess;
  } material;
};

#endif // PROJECT_COFFEE_TABLE_H
