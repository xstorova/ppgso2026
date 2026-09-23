#include "sofa.h"

#include <iostream>
#include <shaders/lighting_vert_glsl.h>
#include <shaders/lighting_frag_glsl.h>

std::unique_ptr<ppgso::Shader> Sofa::shader;
std::unique_ptr<ppgso::Mesh> Sofa::mesh;
std::shared_ptr<ppgso::Texture> Sofa::diffuseTexture;
std::shared_ptr<ppgso::Texture> Sofa::normalTexture;

Sofa::Sofa(glm::vec3 pos, float yawAngleDeg, float scaleMultiplier) {
  if (!shader) {
    shader = std::make_unique<ppgso::Shader>(lighting_vert_glsl, lighting_frag_glsl);
  }

  if (!mesh) {
    std::cout << "[Sofa] Nacitavam model sofa.obj..." << std::endl;
    mesh = std::make_unique<ppgso::Mesh>("sofa.obj");
    std::cout << "[Sofa] Model sofa.obj uspesne nacitany." << std::endl;
  }

  if (!diffuseTexture) {
    try {
      std::cout << "[Sofa] Nacitavam difuznu texturu (Albedo + AO) sofa_diffuse.bmp..." << std::endl;
      diffuseTexture = std::make_shared<ppgso::Texture>(ppgso::image::loadBMP("sofa_diffuse.bmp"));
      std::cout << "[Sofa] Difuzna textura uspesne nacitana." << std::endl;
    } catch (const std::exception &e) {
      std::cerr << "[Sofa] Chyba pri nacitani sofa_diffuse.bmp: " << e.what() << std::endl;
    }
  }

  if (!normalTexture) {
    try {
      std::cout << "[Sofa] Nacitavam normalovu texturu sofa_normal.bmp..." << std::endl;
      normalTexture = std::make_shared<ppgso::Texture>(ppgso::image::loadBMP("sofa_normal.bmp"));
      std::cout << "[Sofa] Normalova textura uspesne nacitana." << std::endl;
    } catch (const std::exception &e) {
      std::cerr << "[Sofa] Chyba pri nacitani sofa_normal.bmp: " << e.what() << std::endl;
    }
  }

  position = pos;
  rotation = glm::vec3(0.0f, glm::radians(yawAngleDeg), 0.0f);
  scale = glm::vec3(scaleMultiplier);

  material.ambient = glm::vec3(0.25f, 0.25f, 0.25f);
  material.diffuse = glm::vec3(0.9f, 0.9f, 0.9f);
  material.specular = glm::vec3(0.15f, 0.15f, 0.15f);
  material.shininess = 24.0f;
}

void Sofa::render(const Scene &scene, float width, float height) {
  if (!mesh || !shader) return;

  shader->use();
  scene.uploadLighting(*shader);

  shader->setUniform("material.ambient", material.ambient);
  shader->setUniform("material.diffuse", material.diffuse);
  shader->setUniform("material.specular", material.specular);
  shader->setUniform("material.shininess", material.shininess);

  if (diffuseTexture) {
    shader->setUniform("Texture", *diffuseTexture, 0);
  }

  if (normalTexture) {
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

void Sofa::renderDepth(const ppgso::Shader &depthShader) {
  if (!mesh) return;
  depthShader.setUniform("ModelMatrix", modelMatrix());
  mesh->render();
}
