#include "floor.h"

#include <shaders/texture_vert_glsl.h>
#include <shaders/texture_frag_glsl.h>

std::unique_ptr<ppgso::Shader> Floor::shader;
std::unique_ptr<ppgso::Mesh> Floor::mesh;
std::unique_ptr<ppgso::Texture> Floor::texture;

Floor::Floor(float topY, float halfWidth, float halfDepth, float thickness) {
  if (!shader) shader = std::make_unique<ppgso::Shader>(texture_vert_glsl, texture_frag_glsl);
  if (!mesh) mesh = std::make_unique<ppgso::Mesh>("cube.obj");
  if (!texture) texture = std::make_unique<ppgso::Texture>(ppgso::image::loadBMP("wood_floor.bmp"));

  // cube.obj ma rozsah -1..1, teda stred kocky je v strede jej hrubky
  float halfThickness = thickness / 2.0f;
  position = glm::vec3(0.0f, topY - halfThickness, 0.0f);
  scale = glm::vec3(halfWidth, halfThickness, halfDepth);
}

void Floor::render(const Camera &camera, float width, float height) {
  shader->use();
  shader->setUniform("Texture", *texture);
  shader->setUniform("ModelMatrix", modelMatrix());
  shader->setUniform("ViewMatrix", camera.viewMatrix());
  shader->setUniform("ProjectionMatrix", camera.projectionMatrix(width, height));
  shader->setUniform("TextureOffset", glm::vec2(0.0f, 0.0f));
  mesh->render();
}