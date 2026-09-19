#ifndef PROJECT_SKYBOX_H
#define PROJECT_SKYBOX_H

#include "../core/object.h"

// [2b] Vyuzitie techniky mapovania na kocku (cube mapping) na vytvorenie Sky-box-u
class Skybox : public Object {
public:
  Skybox();
  ~Skybox() override = default;

  void update(float dt) override;
  void render(const Camera &camera, float width, float height) override;

private:
  static std::unique_ptr<ppgso::Shader> shader;
  static std::unique_ptr<ppgso::Mesh> mesh;
  static GLuint cubemapTexture;
  static bool cubemapLoaded;
};

#endif // PROJECT_SKYBOX_H
