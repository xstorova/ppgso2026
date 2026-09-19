#include "wall.h"

#include <shaders/lighting_vert_glsl.h>
#include <shaders/lighting_frag_glsl.h>
#include "../core/scene.h"
#include <map>

std::unique_ptr<ppgso::Shader> Wall::shader;
std::unique_ptr<ppgso::Mesh> Wall::mesh;
std::shared_ptr<ppgso::Texture> Wall::defaultWhiteTexture;

// Vyrovnavacia pamat (cache) textur, aby sme rovnaky subor nenacitavali opakovane
static std::map<std::string, std::shared_ptr<ppgso::Texture>> textureCache;

static std::shared_ptr<ppgso::Texture> getCachedTexture(const std::string &filename) {
    if (filename.empty()) return nullptr;
    auto it = textureCache.find(filename);
    if (it != textureCache.end()) return it->second;
    try {
        auto tex = std::make_shared<ppgso::Texture>(ppgso::image::loadBMP(filename));
        textureCache[filename] = tex;
        return tex;
    } catch (const std::exception &e) {
        std::cerr << "[Wall] Nepodarilo sa nacitat texturu: " << filename << " (" << e.what() << ")" << std::endl;
        return nullptr;
    }
}

Wall::Wall(float x1, float z1, float x2, float z2,
           float yBottom, float yTop, float thickness,
           float transparency,
           bool useNormalMap,
           const std::string &diffuseFile,
           const std::string &normalMapFile,
           glm::vec2 texScale)
    : transparency(transparency), hasNormalMap(useNormalMap), textureScale(texScale) {
    if (!shader) shader = std::make_unique<ppgso::Shader>(lighting_vert_glsl, lighting_frag_glsl);
    if (!mesh) mesh = std::make_unique<ppgso::Mesh>("cube.obj");
    if (!defaultWhiteTexture) {
        ppgso::Image white(4, 4);
        white.clear({240, 240, 240});
        defaultWhiteTexture = std::make_shared<ppgso::Texture>(std::move(white));
    }

    if (!diffuseFile.empty()) {
        texture = getCachedTexture(diffuseFile);
    }
    if (hasNormalMap && !normalMapFile.empty()) {
        normalMap = getCachedTexture(normalMapFile);
    }

    // Nastavenie materialu
    if (transparency < 1.0f) {
        // [Okno] Sklo: cire, vysoko priehladne s vyraznym zrkadlovym leskom (specular highlight)
        material.ambient = glm::vec3(0.02f, 0.04f, 0.06f);
        material.diffuse = glm::vec3(0.08f, 0.15f, 0.25f);
        material.specular = glm::vec3(1.0f, 1.0f, 1.0f);
        material.shininess = 128.0f;
    } else if (hasNormalMap) {
        // [1b] Hrbolata stena (Normal Mapping):
        // Zvyseny specular a jemny lesk, aby sa plasticky zvyraznili detaily pri pohybe svetla
        material.ambient = glm::vec3(0.18f, 0.18f, 0.18f);
        material.diffuse = glm::vec3(0.9f, 0.88f, 0.85f);
        material.specular = glm::vec3(0.55f, 0.55f, 0.55f);
        material.shininess = 32.0f;
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

    if (texture) {
        shader->setUniform("Texture", *texture, 0);
    } else {
        shader->setUniform("Texture", *defaultWhiteTexture, 0);
    }

    // [1b] Objekt s vyuzitim hrbolatej textury (Normal Mapping)
    if (hasNormalMap && normalMap) {
        shader->setUniform("normalMap", *normalMap, 2);
        glUniform1i(shader->getUniformLocation("useNormalMap"), 1);
    } else {
        glUniform1i(shader->getUniformLocation("useNormalMap"), 0);
    }

    shader->setUniform("Transparency", transparency);
    shader->setUniform("ModelMatrix", modelMatrix());
    shader->setUniform("ViewMatrix", scene.camera.viewMatrix());
    shader->setUniform("ProjectionMatrix", scene.camera.projectionMatrix(widthPx, heightPx));
    shader->setUniform("TextureOffset", glm::vec2(0.0f, 0.0f));
    shader->setUniform("TextureScale", textureScale);

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