#include "object.h"
#include "scene.h"

void Object::render(const Scene &scene, float width, float height) {
  render(scene.camera, width, height);
}
