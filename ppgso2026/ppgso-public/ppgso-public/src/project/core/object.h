#ifndef PROJECT_OBJECT_H
#define PROJECT_OBJECT_H

#include <ppgso/ppgso.h>

#include "camera.h"

class Scene;

class Object {
public:
  Object() = default;
  virtual ~Object() = default;

  virtual void update(float dt) {}
  virtual void render(const Scene &scene, float width, float height);
  virtual void render(const Camera &camera, float width, float height) {}
  virtual void renderDepth(const ppgso::Shader &depthShader) {}

  virtual glm::mat4 modelMatrix() const {
    glm::mat4 matrix = glm::translate(glm::mat4(1.0f), position);
    matrix = glm::rotate(matrix, rotation.x, glm::vec3(1.0f, 0.0f, 0.0f));
    matrix = glm::rotate(matrix, rotation.y, glm::vec3(0.0f, 1.0f, 0.0f));
    matrix = glm::rotate(matrix, rotation.z, glm::vec3(0.0f, 0.0f, 1.0f));
    matrix = glm::scale(matrix, scale);
    return matrix;
  }

  glm::vec3 position{0.0f};
  glm::vec3 rotation{0.0f};
  glm::vec3 scale{1.0f};
};

#endif // PROJECT_OBJECT_H
