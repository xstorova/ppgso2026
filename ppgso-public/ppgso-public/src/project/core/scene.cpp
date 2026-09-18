#include "scene.h"

Scene::Scene() : camera(60.0f, 0.1f, 100.0f) {
  // Placeholder scene initialization.
  // The actual project scene will be populated in later iterations.
}

void Scene::update(float dt) {
  const float speed = 2.0f;

  if (keyboard[GLFW_KEY_W] == GLFW_PRESS || keyboard[GLFW_KEY_W] == GLFW_REPEAT) {
    camera.moveForward(speed * dt);
  }
  if (keyboard[GLFW_KEY_S] == GLFW_PRESS || keyboard[GLFW_KEY_S] == GLFW_REPEAT) {
    camera.moveForward(-speed * dt);
  }
  if (keyboard[GLFW_KEY_D] == GLFW_PRESS || keyboard[GLFW_KEY_D] == GLFW_REPEAT) {
    camera.moveRight(speed * dt);
  }
  if (keyboard[GLFW_KEY_A] == GLFW_PRESS || keyboard[GLFW_KEY_A] == GLFW_REPEAT) {
    camera.moveRight(-speed * dt);
  }
  if (keyboard[GLFW_KEY_Q] == GLFW_PRESS || keyboard[GLFW_KEY_Q] == GLFW_REPEAT) {
    camera.position.y += speed * dt;
    camera.target = camera.position + camera.forward;
  }
  if (keyboard[GLFW_KEY_E] == GLFW_PRESS || keyboard[GLFW_KEY_E] == GLFW_REPEAT) {
    camera.position.y -= speed * dt;
    camera.target = camera.position + camera.forward;
  }
  if (keyboard[GLFW_KEY_LEFT] == GLFW_PRESS || keyboard[GLFW_KEY_LEFT] == GLFW_REPEAT) {
    camera.turn(-1.5f * dt);
  }
  if (keyboard[GLFW_KEY_RIGHT] == GLFW_PRESS || keyboard[GLFW_KEY_RIGHT] == GLFW_REPEAT) {
    camera.turn(1.5f * dt);
  }

  camera.update(dt);

  for (auto &object : objects) {
    if (object) {
      object->update(dt);
    }
  }
}

void Scene::render(float width, float height) {
  for (auto &object : objects) {
    if (object) {
      object->render(camera, width, height);
    }
  }
}

void Scene::handleKey(int key, int action) {
  keyboard[key] = action;
}
