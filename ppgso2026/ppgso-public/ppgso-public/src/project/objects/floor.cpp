#include "floor.h"

#include <shaders/lighting_vert_glsl.h>
#include <shaders/lighting_frag_glsl.h>
#include "../core/scene.h"

std::unique_ptr<ppgso::Shader> Floor::shader;
std::unique_ptr<ppgso::Mesh> Floor::mesh;
std::unique_ptr<ppgso::Texture> Floor::texture;

Floor::Floor(float topY, float halfWidth, float halfDepth, float thickness) {
  if (!shader) shader = std::make_unique<ppgso::Shader>(lighting_vert_glsl, lighting_frag_glsl);
  if (!mesh) mesh = std::make_unique<ppgso::Mesh>("cube.obj");
  if (!texture) texture = std::make_unique<ppgso::Texture>(ppgso::image::loadBMP("wood_floor.bmp"));

  // Drevená podlaha: príjemný difúzny odraz s jemným leskom (specular highlight)
  material.ambient = glm::vec3(0.15f, 0.15f, 0.15f);
  material.diffuse = glm::vec3(0.9f, 0.9f, 0.9f);
  material.specular = glm::vec3(0.4f, 0.35f, 0.3f);
  material.shininess = 32.0f;

  // cube.obj ma rozsah -0.5..0.5 (velkost 1), preto scale = pozadovany rozmer
  position = glm::vec3(0.0f, topY - thickness / 2.0f, 0.0f);
  scale = glm::vec3(halfWidth * 2.0f, thickness, halfDepth * 2.0f);
}

void Floor::render(const Scene &scene, float width, float height) {
  shader->use();
  scene.uploadLighting(*shader);

  shader->setUniform("material.ambient", material.ambient);
  shader->setUniform("material.diffuse", material.diffuse);
  shader->setUniform("material.specular", material.specular);
  shader->setUniform("material.shininess", material.shininess);

  shader->setUniform("Texture", *texture);
  shader->setUniform("Transparency", 1.0f);
  shader->setUniform("ModelMatrix", modelMatrix());
  shader->setUniform("ViewMatrix", scene.camera.viewMatrix());
  shader->setUniform("ProjectionMatrix", scene.camera.projectionMatrix(width, height));
  shader->setUniform("TextureOffset", glm::vec2(0.0f, 0.0f));

  mesh->render();
}