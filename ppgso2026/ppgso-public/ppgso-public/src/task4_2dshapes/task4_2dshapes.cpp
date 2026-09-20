// Task 5 - Draw a 2D shape using polygons and animate it
//        - Encapsulate the shape using a class
//        - Use color_vert/frag shader to display the polygon
//        - Animate the object position, rotation and scale.

#include <iostream>
#include <vector>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtx/matrix_transform_2d.hpp>
#include <glm/gtx/euler_angles.hpp>
#include <ppgso/ppgso.h>

#include <shaders/color_vert_glsl.h>
#include <shaders/color_frag_glsl.h>

#include "glm/gtx/transform.hpp"

const unsigned int SIZE = 512;

// Object to represent 2D OpenGL shape
class Shape {
private:
  // 2D vectors define points/vertices of the shape
  // TODO: Define your shape points
  std::vector<glm::vec3> vetrices = {
    {-0.3f, -0.3f, 0.0f},
      {0.4f, -0.3f, 0.0f},
      {0.4f,  0.0f, 0.0f},
      { 0.0f,  0.0f, 0.0f},
      { 0.0f,  0.7f, 0.0f},
      {-0.3f,  0.7f, 0.0f}
  };

  // Structure representing a triangular face, usually indexes into vertices
  struct Face {
    // TODO: Define your face structure
    GLuint v0, v1, v2;
  };

  // Indices define triangles that index into vertices
  // TODO: Define your mesh indices
  std::vector<Face> mesh = {
    {0, 1, 3},
    {1, 2, 3},
    {0, 3, 5},
    {3, 4, 5}
  };


  // Program to associate with the object
  ppgso::Shader program = {color_vert_glsl, color_frag_glsl};

  // These will hold the data and object buffers
  GLuint vao, vbo, cbo, ibo;
  glm::mat4 modelMatrix{1.0f};
  float timeElapsed = 0.0f;

public:
  // Public attributes that define position, color ..
  glm::vec3 position{0,0,0};
  glm::vec3 rotation{0,0,0};
  glm::vec3 scale{1,1,1};
  glm::vec3 color{1,0,0};

  // Initialize object data buffers
  Shape() {
    // Copy data to OpenGL
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    // Copy positions to gpu
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, vetrices.size() * sizeof(glm::vec3), vetrices.data(), GL_STATIC_DRAW);

    // Set vertex program inputs
    auto position_attrib = program.getAttribLocation("Position");
    glEnableVertexAttribArray(position_attrib);
    glVertexAttribPointer(position_attrib, 3, GL_FLOAT, GL_FALSE, 0, 0);

    // Copy mesh indices to gpu
    glGenBuffers(1, &ibo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, mesh.size() * sizeof(Face), mesh.data(), GL_STATIC_DRAW);

    // Set projection matrices to identity
    program.setUniform("ProjectionMatrix", glm::mat4{1.0f});
    program.setUniform("ViewMatrix", glm::mat4{1.0f});
  };
  // Clean up
  ~Shape() {
    // Delete data from OpenGL
    glDeleteBuffers(1, &ibo);
    glDeleteBuffers(1, &cbo);
    glDeleteBuffers(1, &vbo);
    glDeleteVertexArrays(1, &vao);
  }

  // Set the object transformation matrix
  void update(float time) {
    // TODO: Compute transformation by scaling, rotating and then translating the shape
    timeElapsed = time;

    scale = {0.3f, 0.3f, 1.0f};

    float angularSpeed = 2.0f;
    float direction = (color.g > color.r) ? 1.0f : -1.0f;
    float angle = time * angularSpeed * direction;

    glm::vec3 basePosition;
    if (color.g) {
      basePosition = {0.8f, 0.0f, 0.0f};
    }
    if (color.r) {
      basePosition = {-0.5f, 0.0f, 0.0f};
    }

    // Vytvor maticu orbitálnej rotácie okolo Z osi
    glm::mat4 orbitRotation = glm::rotate(glm::mat4{1.0f}, angle, glm::vec3{0, 0, 1});

    // Použi túto maticu na výpočet aktuálnej pozície po orbite
    glm::vec4 orbitPos = orbitRotation * glm::vec4(basePosition, 1.0f);
    position = glm::vec3(orbitPos);

    //Rotácia okolo vlastnej osi — červený sa točí rýchlejšie
    float selfRotationMultiplier = (color.r > color.g) ? 5.0f : 1.0f;
    rotation.z = angle * selfRotationMultiplier;

    // Modelová matica — poradie je dôležité:
    modelMatrix = glm::mat4{1.0f};
    modelMatrix = glm::translate(modelMatrix, position); // posun po orbite
    modelMatrix = glm::rotate(modelMatrix, rotation.z, glm::vec3{0, 0, 1}); // rotácia okolo vlastnej osi
    modelMatrix = glm::scale(modelMatrix, scale);
  }


  // Draw polygons
  void render(){
    // Update transformation and color uniforms in the shader
    program.use();
    program.setUniform("OverallColor", color);
    program.setUniform("ModelMatrix", modelMatrix);

    glBindVertexArray(vao);
    glDrawElements(GL_TRIANGLES, (GLsizei) mesh.size() * 3, GL_UNSIGNED_INT, 0);
  };
};

class ShapeWindow : public ppgso::Window {
private:
  Shape shape1, shape2;
public:
  ShapeWindow() : Window{"task4_2dshapes", SIZE, SIZE} {
    shape1.color = {0,1,0};
    shape2.color = {1,0,0};
  }

  void onIdle() {
    // Set gray background
    glClearColor(.1f,.1f,.1f,1.0f);
    // Clear depth and color buffers
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Move and Render shape\    // Get time for animation
    auto t = (float) glfwGetTime();

    // TODO: manipuate shape1 and shape2 position to rotate clockwise
    // Update and render each shape
    shape1.update(t);
    shape2.update(t);

    shape1.render();
    shape2.render();
  }
};

int main() {
  // Create our window
  auto window = ShapeWindow{};

  // Main execution loop
  while (window.pollEvents()) {}

  return EXIT_SUCCESS;
}
