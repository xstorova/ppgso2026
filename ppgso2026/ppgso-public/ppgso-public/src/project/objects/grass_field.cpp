#include "grass_field.h"

#include <fstream>
#include <sstream>
#include <iostream>
#include <random>
#include <cmath>

#include <shaders/grass_vert_glsl.h>
#include <shaders/grass_frag_glsl.h>
#include <shaders/grass_shadow_vert_glsl.h>
#include <shaders/grass_shadow_frag_glsl.h>

std::unique_ptr<ppgso::Shader> GrassField::shader;
std::unique_ptr<ppgso::Shader> GrassField::shadowShader;

GrassField::GrassField(int count, float groundY, float areaSize)
    : instanceCount(count),
      indexCount(0),
      elapsedTime(0.0f),
      vao(0), vbo(0), tbo(0), nbo(0), ibo(0), instanceVbo(0),
      textureId(0)
{
  if (!shader) {
    shader = std::make_unique<ppgso::Shader>(grass_vert_glsl, grass_frag_glsl);
  }

  if (!shadowShader) {
    shadowShader = std::make_unique<ppgso::Shader>(grass_shadow_vert_glsl, grass_shadow_frag_glsl);
  }

  // Material travy: ziva zelena s prirodzenym ambientom
  material.ambient  = glm::vec3(0.35f, 0.45f, 0.25f);
  material.diffuse  = glm::vec3(0.90f, 0.95f, 0.70f);
  material.specular = glm::vec3(0.05f, 0.05f, 0.05f);
  material.shininess = 16.0f;

  initMesh();
  initTexture();
  initInstances(groundY, areaSize);
}

GrassField::~GrassField() {
  if (instanceVbo) glDeleteBuffers(1, &instanceVbo);
  if (ibo) glDeleteBuffers(1, &ibo);
  if (nbo) glDeleteBuffers(1, &nbo);
  if (tbo) glDeleteBuffers(1, &tbo);
  if (vbo) glDeleteBuffers(1, &vbo);
  if (vao) glDeleteVertexArrays(1, &vao);
  if (textureId) glDeleteTextures(1, &textureId);
}

void GrassField::initMesh() {
  std::ifstream file("grass06.obj");
  if (!file.is_open()) {
    std::cerr << "[GrassField] ERROR: Could not open grass06.obj!" << std::endl;
    return;
  }

  std::vector<glm::vec3> positions;
  std::vector<glm::vec2> uvs;
  std::vector<unsigned int> indices;

  std::string line;
  float minY = 1e9f;

  while (std::getline(file, line)) {
    if (line.rfind("v ", 0) == 0) {
      std::istringstream ss(line.substr(2));
      glm::vec3 pos;
      ss >> pos.x >> pos.y >> pos.z;
      if (pos.y < minY) minY = pos.y;
      positions.push_back(pos);
    } else if (line.rfind("vt ", 0) == 0) {
      std::istringstream ss(line.substr(3));
      glm::vec2 uv;
      ss >> uv.x >> uv.y;
      uvs.push_back(uv);
    } else if (line.rfind("f ", 0) == 0) {
      std::istringstream ss(line.substr(2));
      std::string part;
      while (ss >> part) {
        size_t slash = part.find('/');
        if (slash != std::string::npos) {
          int vIdx = std::stoi(part.substr(0, slash)) - 1;
          indices.push_back(static_cast<unsigned int>(vIdx));
        } else {
          indices.push_back(static_cast<unsigned int>(std::stoi(part) - 1));
        }
      }
    }
  }

  // Zarovnanie korenov travy na Y = 0.0
  for (auto &pos : positions) {
    pos.y -= minY;
  }

  // Vypocet normal pre vsetky vrcholy
  std::vector<glm::vec3> normals(positions.size(), glm::vec3(0.0f, 1.0f, 0.0f));
  for (size_t i = 0; i + 2 < indices.size(); i += 3) {
    unsigned int i0 = indices[i];
    unsigned int i1 = indices[i + 1];
    unsigned int i2 = indices[i + 2];

    glm::vec3 v0 = positions[i0];
    glm::vec3 v1 = positions[i1];
    glm::vec3 v2 = positions[i2];

    glm::vec3 edge1 = v1 - v0;
    glm::vec3 edge2 = v2 - v0;
    glm::vec3 normal = glm::normalize(glm::cross(edge1, edge2));

    normals[i0] += normal;
    normals[i1] += normal;
    normals[i2] += normal;
  }
  for (auto &n : normals) {
    n = glm::normalize(n);
  }

  indexCount = static_cast<GLsizei>(indices.size());

  glGenVertexArrays(1, &vao);
  glBindVertexArray(vao);

  // VBO: Pozicie (lokacia 0)
  glGenBuffers(1, &vbo);
  glBindBuffer(GL_ARRAY_BUFFER, vbo);
  glBufferData(GL_ARRAY_BUFFER, positions.size() * sizeof(glm::vec3), positions.data(), GL_STATIC_DRAW);
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, nullptr);

  // TBO: UV suradnice (lokacia 1)
  glGenBuffers(1, &tbo);
  glBindBuffer(GL_ARRAY_BUFFER, tbo);
  glBufferData(GL_ARRAY_BUFFER, uvs.size() * sizeof(glm::vec2), uvs.data(), GL_STATIC_DRAW);
  glEnableVertexAttribArray(1);
  glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, nullptr);

  // NBO: Normaly (lokacia 2)
  glGenBuffers(1, &nbo);
  glBindBuffer(GL_ARRAY_BUFFER, nbo);
  glBufferData(GL_ARRAY_BUFFER, normals.size() * sizeof(glm::vec3), normals.data(), GL_STATIC_DRAW);
  glEnableVertexAttribArray(2);
  glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, nullptr);

  // IBO: Indexy
  glGenBuffers(1, &ibo);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);

  glBindVertexArray(0);
  std::cout << "[GrassField] Loaded grass mesh: " << positions.size() << " vertices, " << (indexCount / 3) << " triangles." << std::endl;
}

void GrassField::initTexture() {
  std::ifstream file("grass_diffuse.rgba", std::ios::binary);
  if (!file.is_open()) {
    std::cerr << "[GrassField] WARNING: grass_diffuse.rgba not found!" << std::endl;
    return;
  }

  uint32_t width = 0, height = 0;
  file.read(reinterpret_cast<char*>(&width), sizeof(uint32_t));
  file.read(reinterpret_cast<char*>(&height), sizeof(uint32_t));

  std::vector<uint8_t> data(width * height * 4);
  file.read(reinterpret_cast<char*>(data.data()), data.size());

  glGenTextures(1, &textureId);
  glBindTexture(GL_TEXTURE_2D, textureId);

  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data.data());
  glGenerateMipmap(GL_TEXTURE_2D);

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

  glBindTexture(GL_TEXTURE_2D, 0);
  std::cout << "[GrassField] Loaded grass RGBA texture (" << width << "x" << height << ")." << std::endl;
}

void GrassField::initInstances(float groundY, float areaSize) {
  std::vector<glm::mat4> instanceMatrices;
  instanceMatrices.reserve(instanceCount);

  std::mt19937 rng(1337);
  std::uniform_real_distribution<float> distAngle(0.0f, 360.0f);
  std::uniform_real_distribution<float> distScale(0.045f, 0.075f);
  std::uniform_real_distribution<float> distHeight(0.85f, 1.30f);
  std::uniform_real_distribution<float> distProb(0.0f, 1.0f);

  // Huste pasmo priamo v zornom poli okna (okno je na Z=+9.0, stena siaha po Z=9.25, bezpecna vzdialenost Z >= 11.2)
  std::uniform_real_distribution<float> distDenseX(-14.0f, 14.0f);
  std::uniform_real_distribution<float> distDenseZ(11.2f, 32.0f);

  // Sirsie pasmo pred oknom a prednou stenou
  std::uniform_real_distribution<float> distFrontX(-38.0f, 38.0f);
  std::uniform_real_distribution<float> distFrontZ(11.2f, 60.0f);

  // Okolita krajina
  std::uniform_real_distribution<float> distWideXZ(-65.0f, 65.0f);

  while (static_cast<int>(instanceMatrices.size()) < instanceCount) {
    float x = 0.0f;
    float z = 0.0f;
    float p = distProb(rng);

    if (p < 0.65f) {
      // 65% instancii: husto v primarnom zornom poli okna (od Z=11.2 do 32)
      x = distDenseX(rng);
      z = distDenseZ(rng);
    } else if (p < 0.92f) {
      // 27% instancii: v celom prednom vyhlade pred domom
      x = distFrontX(rng);
      z = distFrontZ(rng);
    } else {
      // 8% instancii: po bokoch
      x = distWideXZ(rng);
      z = distWideXZ(rng);
    }

    // Striktna bezpecnostna zona okolo celeho domu:
    // Dom je od -9 do +9. Polomer trsu travy je ~1.6 jednotky.
    // Teda |x| < 11.0 alebo |z| < 11.0 znamena ze stred trsu je prilis blizko stene a blade by zasiahol dovnutra!
    if (std::abs(x) < 11.0f && std::abs(z) < 11.0f) {
      continue;
    }

    float angle = distAngle(rng);
    float s = distScale(rng);
    float sy = s * distHeight(rng);

    glm::mat4 m = glm::translate(glm::mat4(1.0f), glm::vec3(x, groundY, z));
    m = glm::rotate(m, glm::radians(angle), glm::vec3(0.0f, 1.0f, 0.0f));
    m = glm::scale(m, glm::vec3(s, sy, s));

    instanceMatrices.push_back(m);
  }

  glBindVertexArray(vao);

  glGenBuffers(1, &instanceVbo);
  glBindBuffer(GL_ARRAY_BUFFER, instanceVbo);
  glBufferData(GL_ARRAY_BUFFER, instanceMatrices.size() * sizeof(glm::mat4), instanceMatrices.data(), GL_STATIC_DRAW);

  // Matica 4x4 zabera 4 atributy (lokacie 3, 4, 5, 6)
  std::size_t vec4Size = sizeof(glm::vec4);
  for (unsigned int i = 0; i < 4; ++i) {
    glEnableVertexAttribArray(3 + i);
    glVertexAttribPointer(3 + i, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), reinterpret_cast<void*>(i * vec4Size));
    glVertexAttribDivisor(3 + i, 1); // Posun o 1 zaznam na kazdu instanciu
  }

  glBindVertexArray(0);
  std::cout << "[GrassField] Created " << instanceCount << " grass instances using OpenGL instancing." << std::endl;
}

void GrassField::update(float dt) {
  elapsedTime += dt;
}

void GrassField::render(const Scene &scene, float width, float height) {
  if (!vao || !shader || indexCount == 0) return;

  glDisable(GL_CULL_FACE); // Travnate cepele su obojstranne

  shader->use();
  scene.uploadLighting(*shader);

  shader->setUniform("material.ambient",  material.ambient);
  shader->setUniform("material.diffuse",  material.diffuse);
  shader->setUniform("material.specular", material.specular);
  shader->setUniform("material.shininess", material.shininess);

  shader->setUniform("ViewMatrix", scene.camera.viewMatrix());
  shader->setUniform("ProjectionMatrix", scene.camera.projectionMatrix(width, height));
  shader->setUniform("Time", elapsedTime);

  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, textureId);
  shader->setUniform("Texture", 0);

  glBindVertexArray(vao);
  glDrawElementsInstanced(GL_TRIANGLES, indexCount, GL_UNSIGNED_INT, nullptr, instanceCount);
  glBindVertexArray(0);

  glEnable(GL_CULL_FACE);
}

void GrassField::renderDepth(const ppgso::Shader &depthShader) {
  if (!vao || !shadowShader || indexCount == 0) return;

  GLint loc = glGetUniformLocation(depthShader.getProgram(), "lightSpaceMatrix");
  glm::mat4 lsm(1.0f);
  if (loc != -1) {
    glGetUniformfv(depthShader.getProgram(), loc, glm::value_ptr(lsm));
  }

  shadowShader->use();
  shadowShader->setUniform("LightSpaceMatrix", lsm);

  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, textureId);
  shadowShader->setUniform("Texture", 0);

  glBindVertexArray(vao);
  glDrawElementsInstanced(GL_TRIANGLES, indexCount, GL_UNSIGNED_INT, nullptr, instanceCount);
  glBindVertexArray(0);

  depthShader.use();
}
