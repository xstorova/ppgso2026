#ifndef PROJECT_PEDESTAL_H
#define PROJECT_PEDESTAL_H

#include <memory>
#include <ppgso/ppgso.h>

#include "../core/object.h"
#include "../core/light.h"

// =================================================================================
// [13b] BOD 7 (4b): Objekt v miestnosti vrhajúci dynamický tieň na podlahu
// =================================================================================
class Pedestal : public Object {
public:
  Pedestal(float x = 0.0f, float y = 0.5f, float z = 0.0f);

  void render(const Scene &scene, float width, float height) override;
  void renderDepth(const ppgso::Shader &depthShader) override;

  Material material;

private:
  static std::unique_ptr<ppgso::Shader> shader;
  static std::unique_ptr<ppgso::Mesh> mesh;
  static std::unique_ptr<ppgso::Texture> texture;
};

#endif // PROJECT_PEDESTAL_H
