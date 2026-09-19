#ifndef PROJECT_CEILING_H
#define PROJECT_CEILING_H

#include "../core/object.h"
#include "../core/light.h"

class Ceiling : public Object {
public:
  // bottomY   = presná výška spodnej (viditeľnej) plochy stropu
  // halfWidth = polovica šírky stropu (os X)
  // halfDepth = polovica hĺbky stropu (os Z)
  // thickness = hrúbka stropu
  Ceiling(float bottomY, float halfWidth, float halfDepth, float thickness);

  void render(const Scene &scene, float width, float height) override;

  Material material;

private:
  static std::unique_ptr<ppgso::Shader> shader;
  static std::unique_ptr<ppgso::Mesh> mesh;
  static std::unique_ptr<ppgso::Texture> texture;
};

#endif // PROJECT_CEILING_H
