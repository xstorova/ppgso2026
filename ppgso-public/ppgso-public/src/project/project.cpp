#include <iostream>

#include <ppgso/ppgso.h>

#include "core/scene.h"
#include "objects/floor.h"
#include "objects/skybox.h"
#include "objects/wall.h"
#include "objects/ceiling.h"
#include "objects/light_marker.h"
#include "objects/pedestal.h"

const unsigned int WIDTH = 1280;
const unsigned int HEIGHT = 720;

class ProjectWindow : public ppgso::Window {
private:
  Scene scene;
  bool animate = true;
  bool mouseInitialized = false;
  double lastCursorX = 0.0;
  double lastCursorY = 0.0;

public:
  ProjectWindow() : Window{"project", WIDTH, HEIGHT} {
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    glfwGetCursorPos(window, &lastCursorX, &lastCursorY);

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);
    glDisable(GL_CULL_FACE);

    glClearColor(0.02f, 0.02f, 0.03f, 1.0f);

    // === Presné čísla, žiadne odvodzovanie od iných objektov (skybox) ===
    const float roomHalf      = 9.0f;   // polovica pôdorysu miestnosti
    const float wallDepth     = 0.5f;   // hrúbka stien
    const float floorThickness = 0.4f;  // hrúbka podlahy
    const float floorTopY     = 0.0f;   // presná výška hornej plochy podlahy
    const float wallTopY      = 6.0f;   // presná výška vrchu stien
    const float ceilThickness  = 0.4f;  // hrúbka stropu

    auto sky = std::make_unique<Skybox>();

    scene.camera.position = glm::vec3(0.0f, 2.0f, 12.0f);
    scene.camera.forward = glm::vec3(0.0f, 0.0f, -1.0f);
    scene.camera.target = scene.camera.position + scene.camera.forward;

    scene.objects.push_back(std::move(sky));
    scene.objects.push_back(std::make_unique<Floor>(floorTopY, roomHalf, roomHalf, floorThickness));
    scene.objects.push_back(std::make_unique<Ceiling>(wallTopY, roomHalf, roomHalf, ceilThickness));

    // [1b] Zadná stena s využitím hrboľatej textúry (Normal Mapping):
    scene.objects.push_back(std::make_unique<Wall>(
        -roomHalf, -roomHalf,
         roomHalf, -roomHalf,
         floorTopY, wallTopY,
         wallDepth,
         1.0f,
         true,
         "wall_diffuse.bmp",
         "wall_normal.bmp",
         glm::vec2(4.0f, 2.0f)));

    // Predna stena s vyrezanym otvorom pre okno:
    const float winX1 = -2.5f;
    const float winX2 =  2.5f;
    const float winY1 =  1.2f;
    const float winY2 =  4.8f;

    // 1. Lava cast prednej steny: od -roomHalf po okno (winX1)
    scene.objects.push_back(std::make_unique<Wall>(
        -roomHalf, roomHalf,
         winX1,    roomHalf,
         floorTopY, wallTopY,
         wallDepth,
         1.0f,
         false,
         "wall_diffuse.bmp",
         "",
         glm::vec2(2.0f, 2.0f)));

    // 2. Prava cast prednej steny: od konca okna (winX2) po roomHalf
    scene.objects.push_back(std::make_unique<Wall>(
         winX2,    roomHalf,
         roomHalf, roomHalf,
         floorTopY, wallTopY,
         wallDepth,
         1.0f,
         false,
         "wall_diffuse.bmp",
         "",
         glm::vec2(2.0f, 2.0f)));

    // 3. Spodna cast prednej steny pod oknom (parapet)
    scene.objects.push_back(std::make_unique<Wall>(
         winX1, roomHalf,
         winX2, roomHalf,
         floorTopY, winY1,
         wallDepth,
         1.0f,
         false,
         "wall_diffuse.bmp",
         "",
         glm::vec2(2.0f, 1.0f)));

    // 4. Horna cast prednej steny nad oknom (preklad)
    scene.objects.push_back(std::make_unique<Wall>(
         winX1, roomHalf,
         winX2, roomHalf,
         winY2, wallTopY,
         wallDepth,
         1.0f,
         false,
         "wall_diffuse.bmp",
         "",
         glm::vec2(2.0f, 1.0f)));

    // Lava stena: rohy (-roomHalf, -roomHalf) -> (-roomHalf, roomHalf)
    scene.objects.push_back(std::make_unique<Wall>(
        -roomHalf, -roomHalf,
        -roomHalf,  roomHalf,
         floorTopY, wallTopY,
         wallDepth,
         1.0f,
         false,
         "wall_diffuse.bmp",
         "",
         glm::vec2(4.0f, 2.0f)));

    // Prava stena: rohy (roomHalf, -roomHalf) -> (roomHalf, roomHalf)
    scene.objects.push_back(std::make_unique<Wall>(
         roomHalf, -roomHalf,
         roomHalf,  roomHalf,
         floorTopY, wallTopY,
         wallDepth,
         1.0f,
         false,
         "wall_diffuse.bmp",
         "",
         glm::vec2(4.0f, 2.0f)));

    // [13b] BOD 7 (4b): Objekty v miestnosti vrhajuce tiene (podstavec pod stromom + stlp)
    scene.objects.push_back(std::make_unique<Pedestal>(0.0f, 0.5f, 0.0f));
    scene.objects.push_back(std::make_unique<Pedestal>(2.5f, 0.75f, 2.0f));

    // Vizualny indikator svetla v strede miestnosti (pod stromom)
    scene.objects.push_back(std::make_unique<LightMarker>(0));

    // Priehladne okno osadene presne do vyrezaneho otvoru prednej steny (na konci kvoli alpha blendingu)
    // Zvýšená priehľadnosť (transparency = 0.12f) pre čistý priehľad von do skyboxu
    scene.objects.push_back(std::make_unique<Wall>(
        winX1, roomHalf,
        winX2, roomHalf,
        winY1, winY2,
        0.08f,
        0.12f));

    std::cout << "\n=======================================================\n";
    std::cout << " PPGSO Projekt - Osvetlenie a Shadow-maps (13b)\n";
    std::cout << "=======================================================\n";
    std::cout << " Ovladanie:\n";
    std::cout << "  W, A, S, D, Space, Shift: Pohyb kamery\n";
    std::cout << "  Mys:                      Rozhliadanie sa\n";
    std::cout << "  I / K:                    Posun svetla po osi Z (dopredu/dozadu)\n";
    std::cout << "  J / L:                    Posun svetla po osi X (dolava/doprava)\n";
    std::cout << "  U / O:                    Posun svetla po osi Y (dolu/hore)\n";
    std::cout << "  1, 2, 3, 4:               Zmena farby svetla (tepla, studena, zelena, cervena)\n";
    std::cout << "  B:                        Prepinanie Blinn-Phong / klasicky Phong\n";
    std::cout << "  H:                        Prepinanie HDR tone mapping + gama korekcia\n";
    std::cout << "  X:                        Zapnutie/vypnutie Shadow-maps (4b)\n";
    std::cout << "  V:                        Zapnutie/vypnutie Normal Mapping (1b hrbolata textura)\n";
    std::cout << "  N:                        Zapnutie/vypnutie smeroveho svetla (mesiac)\n";
    std::cout << "  M:                        Zapnutie/vypnutie reflektora (spotlight)\n";
    std::cout << "=======================================================\n\n";
  }

  void onKey(int key, int scanCode, int action, int mods) override {
    scene.handleKey(key, action);

    if (key == GLFW_KEY_P && action == GLFW_PRESS) {
      animate = !animate;
    }
  }

  void onCursorPos(double cursorX, double cursorY) override {
    if (!mouseInitialized) {
      lastCursorX = cursorX;
      lastCursorY = cursorY;
      mouseInitialized = true;
      return;
    }

    const double dx = cursorX - lastCursorX;
    const double dy = cursorY - lastCursorY;
    lastCursorX = cursorX;
    lastCursorY = cursorY;

    scene.camera.turn(-float(dx) * 0.003f);
    scene.camera.lookUp(-float(dy) * 0.003f);
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