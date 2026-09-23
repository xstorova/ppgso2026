#ifndef PROJECT_GRASS_FIELD_H
#define PROJECT_GRASS_FIELD_H

#include <ppgso/ppgso.h>
#include <vector>
#include <memory>
#include <GL/glew.h>

#include "../core/object.h"
#include "../core/scene.h"

class GrassField : public Object {
public:
  GrassField(int count = 5000, float groundY = -0.4f, float areaSize = 150.0f);
  ~GrassField() override;

  void update(float dt) override;
  void render(const Scene &scene, float width, float height) override;
  void renderDepth(const ppgso::Shader &depthShader) override;

private:
  void initMesh();
  void initTexture();
  void initInstances(float groundY, float areaSize);

  int instanceCount;
  GLsizei indexCount;
  float elapsedTime;

  GLuint vao;
  GLuint vbo;
  GLuint tbo;
  GLuint nbo;
  GLuint ibo;
  GLuint instanceVbo;

  GLuint textureId;

  static std::unique_ptr<ppgso::Shader> shader;
  static std::unique_ptr<ppgso::Shader> shadowShader;

  struct {
    glm::vec3 ambient;
    glm::vec3 diffuse;
    glm::vec3 specular;
    float shininess;
  } material;
};

#endif // PROJECT_GRASS_FIELD_H
