#ifndef PROJECT_GROUND_H
#define PROJECT_GROUND_H

#include <ppgso/ppgso.h>
#include <memory>

#include "../core/object.h"
#include "../core/scene.h"

class Ground : public Object {
public:
  Ground(float yLevel = -0.4f,
         float size = 160.0f,
         glm::vec2 textureRepeat = glm::vec2(25.0f, 25.0f));

  void render(const Scene &scene, float width, float height) override;
  void renderDepth(const ppgso::Shader &depthShader) override;

  Material material;
  glm::vec2 textureTiling;

private:
  static std::unique_ptr<ppgso::Shader> shader;
  static std::unique_ptr<ppgso::Mesh> mesh;
  static std::unique_ptr<ppgso::Texture> texture;
};

#endif // PROJECT_GROUND_H
