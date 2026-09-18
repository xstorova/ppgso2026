#include "wall.h"

#include <shaders/texture_vert_glsl.h>
#include <shaders/texture_frag_glsl.h>

std::unique_ptr<ppgso::Shader> Wall::shader;
std::unique_ptr<ppgso::Mesh> Wall::mesh;
std::unique_ptr<ppgso::Texture> Wall::texture;

Wall::Wall(float x1, float z1, float x2, float z2,
           float yBottom, float yTop, float thickness) {
    if (!shader) shader = std::make_unique<ppgso::Shader>(texture_vert_glsl, texture_frag_glsl);
    if (!mesh) mesh = std::make_unique<ppgso::Mesh>("cube.obj");
    if (!texture) texture = std::make_unique<ppgso::Texture>(ppgso::image::loadBMP("wood_floor.bmp"));

    // stred steny medzi zadanymi rohmi a vyskami
    position = glm::vec3((x1 + x2) / 2.0f,
                          (yBottom + yTop) / 2.0f,
                          (z1 + z2) / 2.0f);

    // polovicne rozmery pre cube.obj (-1..1)
    float halfX = (x1 == x2) ? thickness / 2.0f : glm::abs(x2 - x1) / 2.0f;
    float halfZ = (z1 == z2) ? thickness / 2.0f : glm::abs(z2 - z1) / 2.0f;
    float halfY = (yTop - yBottom) / 2.0f;

    scale = glm::vec3(halfX, halfY, halfZ);
}

void Wall::render(const Camera &camera, float widthPx, float heightPx) {
    shader->use();
    shader->setUniform("Texture", *texture);
    shader->setUniform("ModelMatrix", modelMatrix());
    shader->setUniform("ViewMatrix", camera.viewMatrix());
    shader->setUniform("ProjectionMatrix", camera.projectionMatrix(widthPx, heightPx));
    shader->setUniform("TextureOffset", glm::vec2(0.0f, 0.0f));
    mesh->render();
}