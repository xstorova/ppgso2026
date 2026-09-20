#include "ceiling.h"

#include <shaders/lighting_vert_glsl.h>
#include <shaders/lighting_frag_glsl.h>
#include "../core/scene.h"

std::unique_ptr<ppgso::Shader> Ceiling::shader;
std::unique_ptr<ppgso::Mesh> Ceiling::mesh;
std::unique_ptr<ppgso::Texture> Ceiling::texture;

Ceiling::Ceiling(float bottomY, float halfWidth, float halfDepth, float thickness) {
  if (!shader) shader = std::make_unique<ppgso::Shader>(lighting_vert_glsl, lighting_frag_glsl);
  if (!mesh) mesh = std::make_unique<ppgso::Mesh>("cube.obj");
  if (!texture) {
     ppgso::Image white(4, 4);
     white.clear({230, 230, 235});
     texture = std::make_unique<ppgso::Texture>(std::move(white));
  }

  // Material pre strop: matny povrch
  material.ambient = glm::vec3(0.12f, 0.12f, 0.12f);
  material.diffuse = glm::vec3(0.8f, 0.8f, 0.8f);
  material.specular = glm::vec3(0.1f, 0.1f, 0.1f);
  material.shininess = 8.0f;

  // cube.obj ma rozsah -0.5..0.5 (velkost 1), preto scale = pozadovany rozmer
  position = glm::vec3(0.0f, bottomY + thickness / 2.0f, 0.0f);
  scale = glm::vec3(halfWidth * 2.0f, thickness, halfDepth * 2.0f);
}

void Ceiling::render(const Scene &scene, float width, float height) {
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
