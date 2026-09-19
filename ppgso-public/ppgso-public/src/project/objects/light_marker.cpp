#include "light_marker.h"

#include <shaders/color_vert_glsl.h>
#include <shaders/color_frag_glsl.h>
#include "../core/scene.h"

std::unique_ptr<ppgso::Shader> LightMarker::shader;
std::unique_ptr<ppgso::Mesh> LightMarker::mesh;

LightMarker::LightMarker(size_t lightIndex) : lightIndex(lightIndex) {
  if (!shader) shader = std::make_unique<ppgso::Shader>(color_vert_glsl, color_frag_glsl);
  if (!mesh) mesh = std::make_unique<ppgso::Mesh>("sphere.obj");

  scale = glm::vec3(0.25f);
}

void LightMarker::render(const Scene &scene, float width, float height) {
  if (lightIndex >= scene.pointLights.size()) return;
  const auto &light = scene.pointLights[lightIndex];
  if (!light.enabled) return;

  // Pozicia markera sleduje poziciu svetla
  position = light.position;

  shader->use();
  shader->setUniform("OverallColor", glm::min(light.diffuse, glm::vec3(1.0f)));
  shader->setUniform("ModelMatrix", modelMatrix());
  shader->setUniform("ViewMatrix", scene.camera.viewMatrix());
  shader->setUniform("ProjectionMatrix", scene.camera.projectionMatrix(width, height));

  mesh->render();
}
