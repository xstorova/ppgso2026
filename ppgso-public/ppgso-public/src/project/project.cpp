#include <iostream>

#include <ppgso/ppgso.h>

#include "core/scene.h"
#include "objects/floor.h"
#include "objects/skybox.h"
#include "objects/wall.h"

const unsigned int WIDTH = 1280;
const unsigned int HEIGHT = 720;

class ProjectWindow : public ppgso::Window {
private:
  Scene scene;
  bool animate = true;

public:
  ProjectWindow() : Window{"project", WIDTH, HEIGHT} {
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);
    glDisable(GL_CULL_FACE);

    glClearColor(0.1f, 0.1f, 0.15f, 1.0f);

    // === Presné čísla, žiadne odvodzovanie od iných objektov (skybox) ===
    const float roomHalf      = 9.0f;   // polovica pôdorysu miestnosti
    const float wallDepth     = 0.5f;   // hrúbka stien
    const float floorThickness = 0.4f;  // hrúbka podlahy
    const float floorTopY     = 0.0f;   // presná výška hornej plochy podlahy
    const float wallTopY      = 6.0f;   // presná výška vrchu stien

    auto sky = std::make_unique<Skybox>();

    scene.camera.position = glm::vec3(0.0f, 2.0f, 12.0f);
    scene.camera.forward = glm::vec3(0.0f, 0.0f, -1.0f);
    scene.camera.target = scene.camera.position + scene.camera.forward;

    scene.objects.push_back(std::move(sky));
    scene.objects.push_back(std::make_unique<Floor>(floorTopY, roomHalf, roomHalf, floorThickness));

    // Zadná stena: rohy (-roomHalf, -roomHalf) -> (roomHalf, -roomHalf)
    scene.objects.push_back(std::make_unique<Wall>(
        -roomHalf, -roomHalf,
         roomHalf, -roomHalf,
         floorTopY, wallTopY,
         wallDepth));

    // Predná stena: rohy (-roomHalf, roomHalf) -> (roomHalf, roomHalf)
    scene.objects.push_back(std::make_unique<Wall>(
        -roomHalf, roomHalf,
         roomHalf, roomHalf,
         floorTopY, wallTopY,
         wallDepth));

    // Ľavá stena: rohy (-roomHalf, -roomHalf) -> (-roomHalf, roomHalf)
    scene.objects.push_back(std::make_unique<Wall>(
        -roomHalf, -roomHalf,
        -roomHalf,  roomHalf,
         floorTopY, wallTopY,
         wallDepth));

    // Pravá stena: rohy (roomHalf, -roomHalf) -> (roomHalf, roomHalf)
    scene.objects.push_back(std::make_unique<Wall>(
         roomHalf, -roomHalf,
         roomHalf,  roomHalf,
         floorTopY, wallTopY,
         wallDepth));
  }

  void onKey(int key, int scanCode, int action, int mods) override {
    scene.handleKey(key, action);

    if (key == GLFW_KEY_P && action == GLFW_PRESS) {
      animate = !animate;
    }
  }

  void onIdle() override {
    static auto time = (float)glfwGetTime();
    float dt = animate ? (float)glfwGetTime() - time : 0.0f;
    time = (float)glfwGetTime();

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    scene.update(dt);
    scene.render(width, height);
  }
};

int main() {
  ProjectWindow window;

  while (window.pollEvents()) {}

  return EXIT_SUCCESS;
}