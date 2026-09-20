#ifndef PROJECT_SHADOW_MAP_H
#define PROJECT_SHADOW_MAP_H

#include <GL/glew.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

// =================================================================================
// [13b] BOD 7 (4b): Shadow-maps - Správa Framebufferu (FBO) a hĺbkovej textúry
// =================================================================================
class ShadowMap {
public:
  explicit ShadowMap(unsigned int resolution = 2048);
  ~ShadowMap();

  bool init();
  void bindForWriting();
  void bindForReading(GLenum textureUnit) const;
  void unbind(int screenWidth, int screenHeight);

  glm::mat4 computeLightSpaceMatrix(const glm::vec3 &lightPos,
                                   const glm::vec3 &target = glm::vec3(0.0f, 0.0f, 0.0f),
                                   bool perspective = true) const;

  unsigned int getResolution() const { return resolution; }
  GLuint getDepthTexture() const { return depthTexture; }

private:
  unsigned int resolution;
  GLuint fbo{0};
  GLuint depthTexture{0};
};

#endif // PROJECT_SHADOW_MAP_H
