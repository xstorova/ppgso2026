#ifndef PROJECT_CAMERA_H
#define PROJECT_CAMERA_H

#include <ppgso/ppgso.h>

class Camera {
public:
  Camera(float fov = 60.0f, float nearPlane = 0.1f, float farPlane = 100.0f);

  void update(float dt);
  void moveForward(float amount);
  void moveRight(float amount);
  void turn(float angleRadians);
  glm::mat4 viewMatrix() const;
  glm::mat4 projectionMatrix(float width, float height) const;

  glm::vec3 position{0.0f, 1.5f, 5.0f};
  glm::vec3 target{0.0f, 1.0f, 0.0f};
  glm::vec3 forward{0.0f, 0.0f, -1.0f};
  glm::vec3 up{0.0f, 1.0f, 0.0f};

  float fov;
  float nearPlane;
  float farPlane;
};

#endif // PROJECT_CAMERA_H
