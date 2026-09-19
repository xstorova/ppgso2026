#include <cmath>

#include "camera.h"

Camera::Camera(float fov, float nearPlane, float farPlane)
    : fov(fov), nearPlane(nearPlane), farPlane(farPlane) {
  target = position + forward;
}

void Camera::update(float dt) {
  (void)dt;
  target = position + forward;
}

void Camera::moveForward(float amount) {
  position += forward * amount;
  target = position + forward;
}

void Camera::moveRight(float amount) {
  glm::vec3 right = glm::normalize(glm::cross(forward, up));
  position += right * amount;
  target = position + forward;
}

void Camera::turn(float angleRadians) {
  glm::mat4 rotation = glm::rotate(glm::mat4(1.0f), angleRadians, up);
  forward = glm::normalize(glm::vec3(rotation * glm::vec4(forward, 0.0f)));
  target = position + forward;
}

void Camera::lookUp(float angleRadians) {
  glm::vec3 right = glm::normalize(glm::cross(forward, up));
  glm::mat4 rotation = glm::rotate(glm::mat4(1.0f), angleRadians, right);
  glm::vec3 newForward = glm::normalize(glm::vec3(rotation * glm::vec4(forward, 0.0f)));

  if (std::abs(newForward.y) < 0.95f) {
    forward = newForward;
    target = position + forward;
  }
}

glm::mat4 Camera::viewMatrix() const {
  return glm::lookAt(position, target, up);
}

glm::mat4 Camera::projectionMatrix(float width, float height) const {
  return glm::perspective(glm::radians(fov), width / height, nearPlane, farPlane);
}
