#include "coffee_table.h"

#include <iostream>
#include <shaders/lighting_vert_glsl.h>
#include <shaders/lighting_frag_glsl.h>

std::unique_ptr<ppgso::Shader> CoffeeTable::shader;
std::unique_ptr<ppgso::Mesh> CoffeeTable::mesh;
std::unique_ptr<ppgso::Texture> CoffeeTable::texture;

CoffeeTable::CoffeeTable(glm::vec3 pos, float yawAngleDeg, float scaleMultiplier)
    : yawRad(glm::radians(yawAngleDeg)),
      uniformScale(scaleMultiplier),
      // Mesh center: X≈0, Y≈243.68, Z≈-1593.18  →  posunieme ho do originu
      meshCenterOffset(0.0f, -0.26f, 1593.18f)
{
  if (!shader) {
    shader = std::make_unique<ppgso::Shader>(lighting_vert_glsl, lighting_frag_glsl);
  }

  if (!mesh) {
    std::cout << "[CoffeeTable] Nacitavam model..." << std::endl;
    mesh = std::make_unique<ppgso::Mesh>("Art. 16602 - coffee table.obj");
    std::cout << "[CoffeeTable] Model uspesne nacitany." << std::endl;
  }

  if (!texture) {
    // Tmava drevena farba na stol
    ppgso::Image wood(4, 4);
    wood.clear({50, 35, 25});
    texture = std::make_unique<ppgso::Texture>(std::move(wood));
  }

  position = pos;

  // Tmave drevo – kofeovy stolik
  material.ambient  = glm::vec3(0.12f, 0.08f, 0.05f);
  material.diffuse  = glm::vec3(0.35f, 0.22f, 0.14f);
  material.specular = glm::vec3(0.4f, 0.4f, 0.4f);
  material.shininess = 48.0f;
}

glm::mat4 CoffeeTable::modelMatrix() const {
  glm::mat4 m = glm::translate(glm::mat4(1.0f), position);
  m = glm::rotate(m, yawRad, glm::vec3(0.0f, 1.0f, 0.0f));
  m = glm::scale(m, glm::vec3(uniformScale));
  // Posun meshu tak aby jeho spodok (Y_min) bol v Y=0 a stred v XZ bol v originu
  m = glm::translate(m, meshCenterOffset);
  return m;
}

void CoffeeTable::render(const Scene &scene, float width, float height) {
  if (!mesh || !shader) return;

  shader->use();
  scene.uploadLighting(*shader);

  shader->setUniform("material.ambient",  material.ambient);
  shader->setUniform("material.diffuse",  material.diffuse);
  shader->setUniform("material.specular", material.specular);
  shader->setUniform("material.shininess", material.shininess);

  shader->setUniform("Texture", *texture, 0);
  glUniform1i(shader->getUniformLocation("useNormalMap"), 0);

  shader->setUniform("Transparency", 1.0f);
  shader->setUniform("ModelMatrix", modelMatrix());
  shader->setUniform("ViewMatrix", scene.camera.viewMatrix());
  shader->setUniform("ProjectionMatrix", scene.camera.projectionMatrix(width, height));
  shader->setUniform("TextureOffset", glm::vec2(0.0f, 0.0f));
  shader->setUniform("TextureScale", glm::vec2(1.0f, 1.0f));

  mesh->render();
}

void CoffeeTable::renderDepth(const ppgso::Shader &depthShader) {
  if (!mesh) return;
  depthShader.setUniform("ModelMatrix", modelMatrix());
  mesh->render();
}
