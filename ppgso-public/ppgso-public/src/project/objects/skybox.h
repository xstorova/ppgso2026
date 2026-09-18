#ifndef PROJECT_SKYBOX_H
#define PROJECT_SKYBOX_H

#include "../core/object.h"

class Skybox : public Object {
public:
  Skybox();

  void update(float dt) override;
  void render(const Camera &camera, float width, float height) override;

private:
  static std::unique_ptr<ppgso::Shader> shader;
  static std::unique_ptr<ppgso::Mesh> mesh;
  static std::unique_ptr<ppgso::Texture> texture;
  float uvOffset = 0.0f;
};

#endif // PROJECT_SKYBOX_H
