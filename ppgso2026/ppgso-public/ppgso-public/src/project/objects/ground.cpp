#include "ground.h"

#include <shaders/lighting_vert_glsl.h>
#include <shaders/lighting_frag_glsl.h>

std::unique_ptr<ppgso::Shader> Ground::shader;
std::unique_ptr<ppgso::Mesh> Ground::mesh;
std::unique_ptr<ppgso::Texture> Ground::texture;

Ground::Ground(float yLevel, float size, glm::vec2 textureRepeat)
    : textureTiling(textureRepeat)
{
  if (!shader) {
    shader = std::make_unique<ppgso::Shader>(lighting_vert_glsl, lighting_frag_glsl);
  }

  if (!mesh) {
    mesh = std::make_unique<ppgso::Mesh>("cube.obj");
  }

  if (!texture) {
    texture = std::make_unique<ppgso::Texture>(ppgso::image::loadBMP("ground_diffuse.bmp"));
  }

  // Zem / trávnik: prirodzený matný povrch s nízkym odleskom
  material.ambient  = glm::vec3(0.2f, 0.2f, 0.2f);
  material.diffuse  = glm::vec3(0.85f, 0.85f, 0.85f);
  material.specular = glm::vec3(0.08f, 0.08f, 0.08f);
  material.shininess = 8.0f;

  float thickness = 0.5f;
  // Horná plocha zeme bude na úrovni yLevel
  position = glm::vec3(0.0f, yLevel - (thickness / 2.0f), 0.0f);
  scale = glm::vec3(size, thickness, size);
}

void Ground::render(const Scene &scene, float width, float height) {
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
  shader->setUniform("TextureScale", textureTiling);

  mesh->render();
}

void Ground::renderDepth(const ppgso::Shader &depthShader) {
  if (!mesh) return;
  depthShader.setUniform("ModelMatrix", modelMatrix());
  mesh->render();
}
