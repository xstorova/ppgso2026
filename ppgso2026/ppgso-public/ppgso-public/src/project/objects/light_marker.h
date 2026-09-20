#ifndef PROJECT_LIGHT_MARKER_H
#define PROJECT_LIGHT_MARKER_H

#include <memory>
#include <ppgso/ppgso.h>

#include "../core/object.h"

class LightMarker : public Object {
public:
  explicit LightMarker(size_t lightIndex = 0);

  void render(const Scene &scene, float width, float height) override;

private:
  size_t lightIndex;
  static std::unique_ptr<ppgso::Shader> shader;
  static std::unique_ptr<ppgso::Mesh> mesh;
};

#endif // PROJECT_LIGHT_MARKER_H
