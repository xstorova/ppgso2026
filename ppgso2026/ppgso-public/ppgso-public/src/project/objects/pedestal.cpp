#include "pedestal.h"

#include <shaders/lighting_vert_glsl.h>
#include <shaders/lighting_frag_glsl.h>
#include "../core/scene.h"

std::unique_ptr<ppgso::Shader> Pedestal::shader;
std::unique_ptr<ppgso::Mesh> Pedestal::mesh;
std::unique_ptr<ppgso::Texture> Pedestal::texture;

Pedestal::Pedestal(float x, float y, float z) {
  if (!shader) shader = std::make_unique<ppgso::Shader>(lighting_vert_glsl, lighting_frag_glsl);
  if (!mesh) mesh = std::make_unique<ppgso::Mesh>("cube.obj");
  if (!texture) {
    ppgso::Image wood(4, 4);
    wood.clear({140, 95, 60}); // Teplá drevená / kmeňová farba
    texture = std::make_unique<ppgso::Texture>(std::move(wood));
  }

  position = glm::vec3(x, y, z);
  scale = glm::vec3(0.8f, 1.0f, 0.8f);

  material.ambient = glm::vec3(0.2f, 0.15f, 0.1f);
  material.diffuse = glm::vec3(0.8f, 0.6f, 0.4f);
  material.specular = glm::vec3(0.3f, 0.3f, 0.3f);
  material.shininess = 16.0f;
}

void Pedestal::render(const Scene &scene, float width, float height) {
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

// =================================================================================
// [13b] BOD 7 (4b): Vykreslenie geometrie objektu do hĺbkového bufferu (depth pass)
// =================================================================================
void Pedestal::renderDepth(const ppgso::Shader &depthShader) {
  depthShader.setUniform("ModelMatrix", modelMatrix());
  mesh->render();
}
