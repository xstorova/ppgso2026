#include "scene.h"

#include <iostream>
#include <algorithm>

#include <shaders/shadow_vert_glsl.h>
#include <shaders/shadow_frag_glsl.h>

Scene::Scene() : camera(60.0f, 0.1f, 100.0f) {
  // =================================================================================
  // [13b] BOD 1 (3b): Svetlo v strede miestnosti pod stromom (PointLight)
  // [13b] BOD 4 (2b): Tlmenie constant/linear/quadratic a 3 zložky farby svetla
  // =================================================================================
  PointLight centerLight;
  centerLight.enabled = true;
  centerLight.position = glm::vec3(0.0f, 2.5f, 0.0f); // Výška pre pekný dopad tieňov na podlahu
  centerLight.ambient = glm::vec3(0.06f, 0.05f, 0.03f);
  centerLight.diffuse = glm::vec3(1.4f, 1.1f, 0.8f); // Teplé žiarivé svetlo
  centerLight.specular = glm::vec3(1.0f, 0.95f, 0.85f);
  centerLight.constant = 1.0f;
  centerLight.linear = 0.09f;
  centerLight.quadratic = 0.032f;
  pointLights.push_back(centerLight);

  // [13b] BOD 1 (3b): Smerové svetlo (Directional Light - mesačné svetlo cez okno)
  dirLight.enabled = false;
  dirLight.direction = glm::vec3(0.0f, -0.6f, -1.0f);
  dirLight.ambient = glm::vec3(0.03f, 0.03f, 0.06f);
  dirLight.diffuse = glm::vec3(0.25f, 0.25f, 0.45f);
  dirLight.specular = glm::vec3(0.3f, 0.3f, 0.5f);

  // [13b] BOD 1 (3b): Reflektor (Spotlight zo stropu)
  spotLight.enabled = false;
  spotLight.position = glm::vec3(0.0f, 5.5f, 0.0f);
  spotLight.direction = glm::vec3(0.0f, -1.0f, 0.0f);
  spotLight.ambient = glm::vec3(0.0f);
  spotLight.diffuse = glm::vec3(1.2f, 1.2f, 1.2f);
  spotLight.specular = glm::vec3(1.0f, 1.0f, 1.0f);
  spotLight.constant = 1.0f;
  spotLight.linear = 0.09f;
  spotLight.quadratic = 0.032f;
  spotLight.cutOff = glm::cos(glm::radians(15.0f));
  spotLight.outerCutOff = glm::cos(glm::radians(22.5f));

  // =================================================================================
  // [13b] BOD 7 (4b): Shadow-maps - Inicializácia Framebufferu a tieňového shaderu
  // =================================================================================
  shadowMap = std::make_unique<ShadowMap>(2048);
  if (!shadowMap->init()) {
    std::cerr << "[Scene] Nepodarilo sa inicializovat ShadowMap FBO!" << std::endl;
  }
  depthShader = std::make_unique<ppgso::Shader>(shadow_vert_glsl, shadow_frag_glsl);
}

void Scene::update(float dt) {
  const float speed = 3.0f;

  // Pohyb kamery
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
  if (keyboard[GLFW_KEY_SPACE] == GLFW_PRESS || keyboard[GLFW_KEY_SPACE] == GLFW_REPEAT) {
    camera.position.y += speed * dt;
    camera.target = camera.position + camera.forward;
  }
  if (keyboard[GLFW_KEY_LEFT_SHIFT] == GLFW_PRESS || keyboard[GLFW_KEY_LEFT_SHIFT] == GLFW_REPEAT ||
      keyboard[GLFW_KEY_RIGHT_SHIFT] == GLFW_PRESS || keyboard[GLFW_KEY_RIGHT_SHIFT] == GLFW_REPEAT) {
    camera.position.y -= speed * dt;
    camera.target = camera.position + camera.forward;
  }

  // =================================================================================
  // [13b] BOD 2 (1b): Zmena pozície a orientácie zdrojov svetla
  // Ovládanie pozície bodového svetla v miestnosti (I/K = Z, J/L = X, U/O = Y)
  // Pri posune svetla sa tieň prepočítava v reálnom čase!
  // =================================================================================
  if (!pointLights.empty()) {
    const float lightSpeed = 4.0f;
    if (keyboard[GLFW_KEY_I] == GLFW_PRESS || keyboard[GLFW_KEY_I] == GLFW_REPEAT) {
      pointLights[0].position.z -= lightSpeed * dt;
    }
    if (keyboard[GLFW_KEY_K] == GLFW_PRESS || keyboard[GLFW_KEY_K] == GLFW_REPEAT) {
      pointLights[0].position.z += lightSpeed * dt;
    }
    if (keyboard[GLFW_KEY_J] == GLFW_PRESS || keyboard[GLFW_KEY_J] == GLFW_REPEAT) {
      pointLights[0].position.x -= lightSpeed * dt;
    }
    if (keyboard[GLFW_KEY_L] == GLFW_PRESS || keyboard[GLFW_KEY_L] == GLFW_REPEAT) {
      pointLights[0].position.x += lightSpeed * dt;
    }
    if (keyboard[GLFW_KEY_U] == GLFW_PRESS || keyboard[GLFW_KEY_U] == GLFW_REPEAT) {
      pointLights[0].position.y = std::max(0.5f, pointLights[0].position.y - lightSpeed * dt);
    }
    if (keyboard[GLFW_KEY_O] == GLFW_PRESS || keyboard[GLFW_KEY_O] == GLFW_REPEAT) {
      pointLights[0].position.y = std::min(5.8f, pointLights[0].position.y + lightSpeed * dt);
    }
  }

  camera.update(dt);

  for (auto &object : objects) {
    if (object) {
      object->update(dt);
    }
  }
}

void Scene::render(float width, float height) {
  // =================================================================================
  // [13b] BOD 7 (4b): Shadow-maps - 1. PRECHOD: Hĺbkový prechod do tieňovej mapy
  // =================================================================================
  if (shadowsEnabled && shadowMap && depthShader && !pointLights.empty()) {
    glm::vec3 lightPos = pointLights[0].position;
    glm::vec3 target = glm::vec3(lightPos.x, 0.0f, lightPos.z);
    lightSpaceMatrix = shadowMap->computeLightSpaceMatrix(lightPos, target, true);

    shadowMap->bindForWriting();
    depthShader->use();
    depthShader->setUniform("lightSpaceMatrix", lightSpaceMatrix);

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_FRONT); // Odstránenie shadow acne (Peter Panning)

    for (auto &object : objects) {
      if (object) {
        object->renderDepth(*depthShader);
      }
    }

    glCullFace(GL_BACK);
    glDisable(GL_CULL_FACE);
    shadowMap->unbind((int)width, (int)height);
  }

  // =================================================================================
  // [13b] BOD 7 (4b): Shadow-maps - 2. PRECHOD: Finálne vykreslenie scény
  // =================================================================================
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  if (shadowMap) {
    shadowMap->bindForReading(GL_TEXTURE1);
  }

  for (auto &object : objects) {
    if (object) {
      object->render(*this, width, height);
    }
  }
}

void Scene::handleKey(int key, int action) {
  keyboard[key] = action;

  if (action == GLFW_PRESS) {
    // [13b] BOD 2 (1b): Zmena odtieňov farby osvetlenia (klávesy 1, 2, 3, 4)
    if (!pointLights.empty()) {
      if (key == GLFW_KEY_1) {
        pointLights[0].diffuse = glm::vec3(1.4f, 1.1f, 0.8f);
        pointLights[0].specular = glm::vec3(1.0f, 0.95f, 0.85f);
        std::cout << "[Svetlo] Zmena odtiena: Tepla biela / ohen" << std::endl;
      } else if (key == GLFW_KEY_2) {
        pointLights[0].diffuse = glm::vec3(1.2f, 1.25f, 1.4f);
        pointLights[0].specular = glm::vec3(1.0f, 1.0f, 1.0f);
        std::cout << "[Svetlo] Zmena odtiena: Studena biela" << std::endl;
      } else if (key == GLFW_KEY_3) {
        pointLights[0].diffuse = glm::vec3(0.4f, 1.4f, 0.6f);
        pointLights[0].specular = glm::vec3(0.6f, 1.0f, 0.7f);
        std::cout << "[Svetlo] Zmena odtiena: Smaragdova zelena" << std::endl;
      } else if (key == GLFW_KEY_4) {
        pointLights[0].diffuse = glm::vec3(1.5f, 0.5f, 0.2f);
        pointLights[0].specular = glm::vec3(1.0f, 0.6f, 0.3f);
        std::cout << "[Svetlo] Zmena odtiena: Cervenkasty zapad slnka" << std::endl;
      }
    }

    // [13b] BOD 5 (1b): Blinn-Phong vs Phong
    if (key == GLFW_KEY_B) {
      useBlinnPhong = !useBlinnPhong;
      std::cout << "[Model] Osvetlenie prepnute na: " << (useBlinnPhong ? "Blinn-Phong" : "Klasicky Phong") << std::endl;
    }

    // [13b] BOD 6 (1b): HDR tone mapping + gama korekcia
    if (key == GLFW_KEY_H) {
      useHDR = !useHDR;
      std::cout << "[HDR] Tone mapping a gama korekcia: " << (useHDR ? "ZAPNUTE" : "VYPNUTE") << std::endl;
    }

    // [13b] BOD 7 (4b): Zapnutie / vypnutie tieňov (kláves X)
    if (key == GLFW_KEY_X) {
      shadowsEnabled = !shadowsEnabled;
      std::cout << "[Tiene] Shadow-maps (4b): " << (shadowsEnabled ? "ZAPNUTE" : "VYPNUTE") << std::endl;
    }

    // Zapínanie smerového svetla (N) a reflektora (M)
    if (key == GLFW_KEY_N) {
      dirLight.enabled = !dirLight.enabled;
      std::cout << "[Svetlo] Smerove svetlo (mesacne svetlo): " << (dirLight.enabled ? "ZAPNUTE" : "VYPNUTE") << std::endl;
    }
    if (key == GLFW_KEY_M) {
      spotLight.enabled = !spotLight.enabled;
      std::cout << "[Svetlo] Reflektor (spotlight): " << (spotLight.enabled ? "ZAPNUTE" : "VYPNUTE") << std::endl;
    }

    // [1b] Prepinanie hrbolatej textury (Normal Mapping) (klaves V)
    if (key == GLFW_KEY_V) {
      normalMapEnabled = !normalMapEnabled;
      std::cout << "[Normal Mapping] (1b) Hrbolata textura: " << (normalMapEnabled ? "ZAPNUTE" : "VYPNUTE") << std::endl;
    }
  }
}

void Scene::uploadLighting(const ppgso::Shader &shader) const {
  shader.use();

  shader.setUniform("CameraPosition", camera.position);

  // =================================================================================
  // [1b] Normal Mapping - Globalne povolenie a predvolene hodnoty pre objekty
  // =================================================================================
  glUniform1i(shader.getUniformLocation("normalMapGlobalEnabled"), normalMapEnabled ? 1 : 0);
  glUniform1i(shader.getUniformLocation("useNormalMap"), 0);
  shader.setUniform("TextureScale", glm::vec2(1.0f, 1.0f));

  // =================================================================================
  // [13b] BOD 7 (4b): Odovzdanie tieňovej matice, tieňovej textúry a prepínača do shaderu
  // =================================================================================
  shader.setUniform("lightSpaceMatrix", lightSpaceMatrix);
  glUniform1i(shader.getUniformLocation("shadowsEnabled"), shadowsEnabled ? 1 : 0);
  glUniform1i(shader.getUniformLocation("shadowMap"), 1); // GL_TEXTURE1

  // Smerové svetlo
  glUniform1i(shader.getUniformLocation("dirLight.enabled"), dirLight.enabled ? 1 : 0);
  shader.setUniform("dirLight.direction", dirLight.direction);
  shader.setUniform("dirLight.ambient", dirLight.ambient);
  shader.setUniform("dirLight.diffuse", dirLight.diffuse);
  shader.setUniform("dirLight.specular", dirLight.specular);

  // Bodové svetlá
  int count = std::min((int)pointLights.size(), 4);
  glUniform1i(shader.getUniformLocation("numPointLights"), count);
  for (int i = 0; i < count; ++i) {
    std::string prefix = "pointLights[" + std::to_string(i) + "].";
    glUniform1i(shader.getUniformLocation(prefix + "enabled"), pointLights[i].enabled ? 1 : 0);
    shader.setUniform(prefix + "position", pointLights[i].position);
    shader.setUniform(prefix + "ambient", pointLights[i].ambient);
    shader.setUniform(prefix + "diffuse", pointLights[i].diffuse);
    shader.setUniform(prefix + "specular", pointLights[i].specular);
    shader.setUniform(prefix + "constant", pointLights[i].constant);
    shader.setUniform(prefix + "linear", pointLights[i].linear);
    shader.setUniform(prefix + "quadratic", pointLights[i].quadratic);
  }

  // Reflektor (spotlight)
  glUniform1i(shader.getUniformLocation("spotLight.enabled"), spotLight.enabled ? 1 : 0);
  shader.setUniform("spotLight.position", spotLight.position);
  shader.setUniform("spotLight.direction", spotLight.direction);
  shader.setUniform("spotLight.ambient", spotLight.ambient);
  shader.setUniform("spotLight.diffuse", spotLight.diffuse);
  shader.setUniform("spotLight.specular", spotLight.specular);
  shader.setUniform("spotLight.constant", spotLight.constant);
  shader.setUniform("spotLight.linear", spotLight.linear);
  shader.setUniform("spotLight.quadratic", spotLight.quadratic);
  shader.setUniform("spotLight.cutOff", spotLight.cutOff);
  shader.setUniform("spotLight.outerCutOff", spotLight.outerCutOff);

  // Nastavenia osvetľovacieho modelu
  glUniform1i(shader.getUniformLocation("useBlinnPhong"), useBlinnPhong ? 1 : 0);
  glUniform1i(shader.getUniformLocation("useHDR"), useHDR ? 1 : 0);
  shader.setUniform("gamma", gamma);
}
