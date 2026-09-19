#include "wall.h"

#include <shaders/lighting_vert_glsl.h>
#include <shaders/lighting_frag_glsl.h>
#include "../core/scene.h"

std::unique_ptr<ppgso::Shader> Wall::shader;
std::unique_ptr<ppgso::Mesh> Wall::mesh;
std::unique_ptr<ppgso::Texture> Wall::texture;

Wall::Wall(float x1, float z1, float x2, float z2,
           float yBottom, float yTop, float thickness,
           float transparency)
    : transparency(transparency) {
    if (!shader) shader = std::make_unique<ppgso::Shader>(lighting_vert_glsl, lighting_frag_glsl);
    if (!mesh) mesh = std::make_unique<ppgso::Mesh>("cube.obj");
    if (!texture) {
        ppgso::Image white(4, 4);
        white.clear({240, 240, 240});
        texture = std::make_unique<ppgso::Texture>(std::move(white));
    }

    // Material pre stenu: matny povrch so slabym specular leskom
    if (transparency < 1.0f) {
        // Sklo: jemny modrasty leskly material
        material.ambient = glm::vec3(0.1f, 0.2f, 0.3f);
        material.diffuse = glm::vec3(0.4f, 0.7f, 0.9f);
        material.specular = glm::vec3(0.9f, 0.95f, 1.0f);
        material.shininess = 64.0f;
    } else {
        material.ambient = glm::vec3(0.15f, 0.15f, 0.15f);
        material.diffuse = glm::vec3(0.85f, 0.85f, 0.85f);
        material.specular = glm::vec3(0.2f, 0.2f, 0.2f);
        material.shininess = 16.0f;
    }

    // stred steny medzi zadanymi rohmi a vyskami
    position = glm::vec3((x1 + x2) / 2.0f,
                         (yBottom + yTop) / 2.0f,
                         (z1 + z2) / 2.0f);

    // cube.obj ma rozsah -0.5..0.5 (velkost 1), preto scale = pozadovany rozmer
    float sizeX = (x1 == x2) ? thickness : glm::abs(x2 - x1);
    float sizeZ = (z1 == z2) ? thickness : glm::abs(z2 - z1);
    float sizeY = (yTop - yBottom);

    scale = glm::vec3(sizeX, sizeY, sizeZ);
}

void Wall::render(const Scene &scene, float widthPx, float heightPx) {
    shader->use();
    scene.uploadLighting(*shader);

    shader->setUniform("material.ambient", material.ambient);
    shader->setUniform("material.diffuse", material.diffuse);
    shader->setUniform("material.specular", material.specular);
    shader->setUniform("material.shininess", material.shininess);

    shader->setUniform("Texture", *texture);
    shader->setUniform("Transparency", transparency);
    shader->setUniform("ModelMatrix", modelMatrix());
    shader->setUniform("ViewMatrix", scene.camera.viewMatrix());
    shader->setUniform("ProjectionMatrix", scene.camera.projectionMatrix(widthPx, heightPx));
    shader->setUniform("TextureOffset", glm::vec2(0.0f, 0.0f));

    if (transparency < 1.0f) {
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glDepthMask(GL_FALSE);
    }
    mesh->render();
    if (transparency < 1.0f) {
        glDepthMask(GL_TRUE);
        glDisable(GL_BLEND);
    }
}

void Wall::renderDepth(const ppgso::Shader &depthShader) {
    if (transparency < 0.5f) return; // Priehľadné okno nevrhá plný nepriehľadný tieň
    depthShader.setUniform("ModelMatrix", modelMatrix());
    mesh->render();
}