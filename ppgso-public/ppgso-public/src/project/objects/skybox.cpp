#include "skybox.h"

#include <shaders/texture_vert_glsl.h>
#include <shaders/texture_frag_glsl.h>

std::unique_ptr<ppgso::Shader> Skybox::shader;
std::unique_ptr<ppgso::Mesh> Skybox::mesh;
std::unique_ptr<ppgso::Texture> Skybox::texture;

Skybox::Skybox() {
  if (!shader) shader = std::make_unique<ppgso::Shader>(texture_vert_glsl, texture_frag_glsl);
  if (!mesh) mesh = std::make_unique<ppgso::Mesh>("cube.obj");
  if (!texture) texture = std::make_unique<ppgso::Texture>(ppgso::image::loadBMP("stars.bmp"));

  position = glm::vec3(0.0f, 0.0f, 0.0f);
  scale = glm::vec3(40.0f, 30.0f, 40.0f);
}

void Skybox::update(float dt) {
  uvOffset += dt * 0.04f;
}

void Skybox::render(const Camera &camera, float width, float height) {
  shader->use();
  shader->setUniform("Texture", *texture);
  shader->setUniform("ModelMatrix", modelMatrix());
  shader->setUniform("ViewMatrix", camera.viewMatrix());
  shader->setUniform("ProjectionMatrix", camera.projectionMatrix(width, height));
  shader->setUniform("TextureOffset", glm::vec2(uvOffset, uvOffset * 0.5f));

  glDisable(GL_CULL_FACE);
  glDepthMask(GL_FALSE);
  mesh->render();
  glDepthMask(GL_TRUE);
  glEnable(GL_CULL_FACE);
}
