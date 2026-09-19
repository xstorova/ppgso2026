// =================================================================================
// [2b] Vyuzitie techniky mapovania na kocku (cube mapping) na vytvorenie Sky-box-u
// - Nacitanie 6 stran kocky (posx, negx, posy, negy, posz, negz)
// - Vytvorenie a konfiguracia OpenGL GL_TEXTURE_CUBE_MAP
// - Vykreslenie Sky-boxu pomocou kocky (cube.obj) s GL_LEQUAL hlbkou
// =================================================================================
#include "skybox.h"

#include <iostream>
#include <vector>
#include <shaders/skybox_vert_glsl.h>
#include <shaders/skybox_frag_glsl.h>

std::unique_ptr<ppgso::Shader> Skybox::shader;
std::unique_ptr<ppgso::Mesh> Skybox::mesh;
GLuint Skybox::cubemapTexture = 0;
bool Skybox::cubemapLoaded = false;

Skybox::Skybox() {
  if (!shader) shader = std::make_unique<ppgso::Shader>(skybox_vert_glsl, skybox_frag_glsl);
  if (!mesh) mesh = std::make_unique<ppgso::Mesh>("cube.obj");

  // [2b] Vytvorenie a inicializacia Cube Mapy v OpenGL
  if (!cubemapLoaded) {
    glGenTextures(1, &cubemapTexture);
    glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture);

    const std::vector<std::string> faces = {
      "skybox_posx.bmp", // +X (Right)
      "skybox_negx.bmp", // -X (Left)
      "skybox_posy.bmp", // +Y (Top / Up)
      "skybox_negy.bmp", // -Y (Bottom / Down)
      "skybox_posz.bmp", // +Z (Back)
      "skybox_negz.bmp"  // -Z (Front)
    };

    for (unsigned int i = 0; i < faces.size(); i++) {
      try {
        ppgso::Image img = ppgso::image::loadBMP(faces[i]);
        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
                     0, GL_RGB, img.width, img.height, 0, GL_RGB, GL_UNSIGNED_BYTE,
                     img.getFramebuffer().data());
      } catch (const std::exception &e) {
        std::cerr << "Failed to load cubemap face: " << faces[i] << " - " << e.what() << std::endl;
      }
    }

    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    cubemapLoaded = true;
  }
}

void Skybox::update(float dt) {
}

void Skybox::render(const Camera &camera, float width, float height) {
  glDisable(GL_CULL_FACE);
  glDepthFunc(GL_LEQUAL);
  glDepthMask(GL_FALSE);

  shader->use();
  shader->setUniform("ProjectionMatrix", camera.projectionMatrix(width, height));
  shader->setUniform("ViewMatrix", camera.viewMatrix());
  shader->setUniform("skyboxYOffset", -0.45f); // Zaporne: posunie izbu HORE v skyboxe

  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture);
  shader->setUniform("skybox", 0);

  mesh->render();

  glDepthMask(GL_TRUE);
  glDepthFunc(GL_LESS);
  glDisable(GL_CULL_FACE);
}

