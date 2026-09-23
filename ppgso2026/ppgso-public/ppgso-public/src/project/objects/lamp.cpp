#include "lamp.h"

#include <iostream>
#include <shaders/lighting_vert_glsl.h>
#include <shaders/lighting_frag_glsl.h>
#include "../core/scene.h"

std::unique_ptr<ppgso::Shader> Lamp::shader;
std::unique_ptr<ppgso::Mesh> Lamp::mesh;
std::shared_ptr<ppgso::Texture> Lamp::diffuseTexture;
std::shared_ptr<ppgso::Texture> Lamp::normalTexture;

Lamp::Lamp(glm::vec3 pos, float yawAngleDeg, float scaleMultiplier)
    : yawRad(glm::radians(yawAngleDeg)),
      uniformScale(scaleMultiplier),
      // Vrch montážnej dosky lampy je na Y ≈ 171.29 cm. Posunieme ho do Y = 0, aby presne priliehal k stropu.
      meshMountOffset(0.0f, -171.29f, 0.0f)
{
  if (!shader) {
    shader = std::make_unique<ppgso::Shader>(lighting_vert_glsl, lighting_frag_glsl);
  }

  if (!mesh) {
    std::cout << "[Lamp] Nacitavam model lamp.obj..." << std::endl;
    try {
      mesh = std::make_unique<ppgso::Mesh>("lamp.obj");
      std::cout << "[Lamp] Model lamp.obj uspesne nacitany." << std::endl;
    } catch (const std::exception &e) {
      std::cerr << "[Lamp] Chyba pri nacitani lamp.obj, skusam Lampara techo.obj: " << e.what() << std::endl;
      mesh = std::make_unique<ppgso::Mesh>("Lampara techo.obj");
    }
  }

  if (!diffuseTexture) {
    try {
      std::cout << "[Lamp] Nacitavam difuznu texturu lamp_diffuse.bmp..." << std::endl;
      diffuseTexture = std::make_shared<ppgso::Texture>(ppgso::image::loadBMP("lamp_diffuse.bmp"));
      std::cout << "[Lamp] Difuzna textura uspesne nacitana." << std::endl;
    } catch (const std::exception &e) {
      std::cerr << "[Lamp] Chyba pri nacitani lamp_diffuse.bmp: " << e.what() << std::endl;
    }
  }

  if (!normalTexture) {
    try {
      std::cout << "[Lamp] Nacitavam normalovu texturu lamp_normal.bmp..." << std::endl;
      normalTexture = std::make_shared<ppgso::Texture>(ppgso::image::loadBMP("lamp_normal.bmp"));
      std::cout << "[Lamp] Normalova textura uspesne nacitana." << std::endl;
    } catch (const std::exception &e) {
      std::cerr << "[Lamp] Chyba pri nacitani lamp_normal.bmp: " << e.what() << std::endl;
    }
  }

  position = pos;

  // Kovovy elegantny matny/pololeskly material lampy
  material.ambient  = glm::vec3(0.2f, 0.2f, 0.2f);
  material.diffuse  = glm::vec3(0.85f, 0.85f, 0.85f);
  material.specular = glm::vec3(0.4f, 0.4f, 0.4f);
  material.shininess = 32.0f;
}

glm::mat4 Lamp::modelMatrix() const {
  glm::mat4 m = glm::translate(glm::mat4(1.0f), position);
  m = glm::rotate(m, yawRad, glm::vec3(0.0f, 1.0f, 0.0f));
  m = glm::scale(m, glm::vec3(uniformScale));
  // Posun meshu tak, aby vrchny upevnovaci bod dosadol priamo na strop
  m = glm::translate(m, meshMountOffset);
  return m;
}

void Lamp::render(const Scene &scene, float width, float height) {
  if (!mesh || !shader) return;

  shader->use();
  scene.uploadLighting(*shader);

  shader->setUniform("material.ambient",  material.ambient);
  shader->setUniform("material.diffuse",  material.diffuse);
  shader->setUniform("material.specular", material.specular);
  shader->setUniform("material.shininess", material.shininess);

  if (diffuseTexture) {
    shader->setUniform("Texture", *diffuseTexture, 0);
  }

  if (normalTexture && scene.normalMapEnabled) {
    shader->setUniform("normalMap", *normalTexture, 2);
    glUniform1i(shader->getUniformLocation("useNormalMap"), 1);
  } else {
    glUniform1i(shader->getUniformLocation("useNormalMap"), 0);
  }

  shader->setUniform("Transparency", 1.0f);
  shader->setUniform("ModelMatrix", modelMatrix());
  shader->setUniform("ViewMatrix", scene.camera.viewMatrix());
  shader->setUniform("ProjectionMatrix", scene.camera.projectionMatrix(width, height));
  shader->setUniform("TextureOffset", glm::vec2(0.0f, 0.0f));
  shader->setUniform("TextureScale", glm::vec2(1.0f, 1.0f));

  mesh->render();
}

void Lamp::renderDepth(const ppgso::Shader &depthShader) {
  if (!mesh) return;
  depthShader.setUniform("ModelMatrix", modelMatrix());
  mesh->render();
}
