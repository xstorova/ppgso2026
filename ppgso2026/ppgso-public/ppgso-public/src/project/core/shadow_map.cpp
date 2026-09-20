#include "shadow_map.h"

#include <iostream>

ShadowMap::ShadowMap(unsigned int resolution) : resolution(resolution) {}

ShadowMap::~ShadowMap() {
  if (depthTexture) glDeleteTextures(1, &depthTexture);
  if (fbo) glDeleteFramebuffers(1, &fbo);
}

bool ShadowMap::init() {
  // [13b] BOD 7 (4b): Vytvorenie hĺbkovej textúry
  glGenTextures(1, &depthTexture);
  glBindTexture(GL_TEXTURE_2D, depthTexture);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, (GLsizei)resolution, (GLsizei)resolution,
               0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

  // Zamedzenie zatieneniu mimo zorného poľa svetla (clamp to border s bielou farbou)
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
  float borderColor[] = {1.0f, 1.0f, 1.0f, 1.0f};
  glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);

  // [13b] BOD 7 (4b): Vytvorenie Framebufferu (FBO) a pripojenie hĺbkovej textúry
  glGenFramebuffers(1, &fbo);
  glBindFramebuffer(GL_FRAMEBUFFER, fbo);
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthTexture, 0);

  // Tento framebuffer nepotrebuje farebný buffer (iba hĺbkový)
  glDrawBuffer(GL_NONE);
  glReadBuffer(GL_NONE);

  GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
  if (status != GL_FRAMEBUFFER_COMPLETE) {
    std::cerr << "[ShadowMap] Chyba: Framebuffer nie je kompletny! Status: " << status << std::endl;
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    return false;
  }

  glBindFramebuffer(GL_FRAMEBUFFER, 0);
  return true;
}

void ShadowMap::bindForWriting() {
  glViewport(0, 0, (GLsizei)resolution, (GLsizei)resolution);
  glBindFramebuffer(GL_FRAMEBUFFER, fbo);
  glClear(GL_DEPTH_BUFFER_BIT);
}

void ShadowMap::unbind(int screenWidth, int screenHeight) {
  glBindFramebuffer(GL_FRAMEBUFFER, 0);
  glViewport(0, 0, screenWidth, screenHeight);
}

void ShadowMap::bindForReading(GLenum textureUnit) const {
  glActiveTexture(textureUnit);
  glBindTexture(GL_TEXTURE_2D, depthTexture);
}

glm::mat4 ShadowMap::computeLightSpaceMatrix(const glm::vec3 &lightPos,
                                            const glm::vec3 &target,
                                            bool perspective) const {
  glm::mat4 lightProjection;
  glm::vec3 up = glm::vec3(0.0f, 0.0f, -1.0f);
  if (glm::abs(lightPos.x - target.x) < 0.001f && glm::abs(lightPos.z - target.z) < 0.001f) {
    up = glm::vec3(0.0f, 0.0f, -1.0f);
  } else {
    up = glm::vec3(0.0f, 1.0f, 0.0f);
  }

  glm::mat4 lightView = glm::lookAt(lightPos, target, up);

  if (perspective) {
    // Perspektívna projekcia pre bodové svetlo / reflektor
    lightProjection = glm::perspective(glm::radians(120.0f), 1.0f, 0.2f, 30.0f);
  } else {
    // Ortografická projekcia pre smerové svetlo
    lightProjection = glm::ortho(-10.0f, 10.0f, -10.0f, 10.0f, 0.1f, 30.0f);
  }

  return lightProjection * lightView;
}
