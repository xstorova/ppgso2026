#ifndef PROJECT_FLOOR_H
#define PROJECT_FLOOR_H

#include "../core/object.h"
#include "../core/light.h"

class Floor : public Object {
public:
  // topY      = presná výška hornej (chodiacej) plochy podlahy
  // halfWidth = polovica šírky podlahy (os X)
  // halfDepth = polovica hĺbky podlahy (os Z)
  // thickness = hrúbka podlahy
  Floor(float topY, float halfWidth, float halfDepth, float thickness);

  void render(const Scene &scene, float width, float height) override;

  Material material;

private:
  static std::unique_ptr<ppgso::Shader> shader;
  static std::unique_ptr<ppgso::Mesh> mesh;
  static std::unique_ptr<ppgso::Texture> texture;
};

#endif // PROJECT_FLOOR_H