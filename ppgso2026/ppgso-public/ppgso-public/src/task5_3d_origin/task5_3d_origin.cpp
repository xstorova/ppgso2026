// Task 5 - Draw an origin with 3 axis and an animated cube rotating around one of the axis
//        - Encapsulate the Cube using a class, it will be used for the rotating object and the axis
//        - Use color_vert/frag shader to display the polygon
//        - Animate the rotation of the all 3 axis and the cube.

#include <iostream>
#include <vector>
#include <map>
#include <cmath>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtx/matrix_transform_2d.hpp>
#include <glm/gtx/euler_angles.hpp>
#include <ppgso/ppgso.h>

#include <shaders/color_vert_glsl.h>
#include <shaders/color_frag_glsl.h>

const unsigned int SIZE = 512;

// Object to represent a 3D cube
class Cube {
private:
    // 2D vectors define points/vertices of the shape
	 // TODO: Define cube vertices
    std::vector<glm::vec3> vertices = {
        {-0.5f, -0.5f, -0.5f},
        { 0.5f, -0.5f, -0.5f},
        { 0.5f,  0.5f, -0.5f},
        {-0.5f,  0.5f, -0.5f},
        {-0.5f, -0.5f,  0.5f},
        { 0.5f, -0.5f,  0.5f},
        { 0.5f,  0.5f,  0.5f},
        {-0.5f,  0.5f,  0.5f},
    };

    // Structure representing a triangular face
    struct Face {
        GLuint v0, v1, v2;
    };

    // Indices define triangles that index into vertices
	 // TODO: Define cube indices
    // TODO: Define cube indices
    std::vector<Face> indices = {
        {0, 1, 2}, {2, 3, 0}, // Back
        {4, 5, 6}, {6, 7, 4}, // Front
        {0, 4, 7}, {7, 3, 0}, // Left
        {1, 5, 6}, {6, 2, 1}, // Right
        {3, 2, 6}, {6, 7, 3}, // Top
        {0, 1, 5}, {5, 4, 0}  // Bottom
    };

    // Program to associate with the object
    ppgso::Shader program = {color_vert_glsl, color_frag_glsl};

    // These will hold the data and object buffers
    GLuint vao, vbo, cbo, ibo;
    glm::mat4 modelMatrix;
    glm::mat4 viewMatrix;

public:
    // Public attributes that define position, color ..
    glm::vec3 position{0,0,0};
    glm::vec3 rotation{0,0,0};
    glm::vec3 scale{1,1,1};
    glm::vec3 color{1,0,0};


    // Initialize object data buffers
    Cube() {
        // Copy data to OpenGL
        glGenVertexArrays(1, &vao);
        glBindVertexArray(vao);

        // Copy positions to gpu
        glGenBuffers(1, &vbo);
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(glm::vec3), vertices.data(), GL_STATIC_DRAW);

        // Set vertex program inputs
        auto position_attrib = program.getAttribLocation("Position");
        glEnableVertexAttribArray(position_attrib);
        glVertexAttribPointer(position_attrib, 3, GL_FLOAT, GL_FALSE, 0, 0);

        // Copy indices to gpu
        glGenBuffers(1, &ibo);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(Face), indices.data(), GL_STATIC_DRAW);

        // Set projection matrices to identity
        program.setUniform("ProjectionMatrix", glm::perspective((ppgso::PI / 180.f) * 60.0f, 1.0f, 0.1f, 100.0f));

        program.setUniform("ViewMatrix", viewMatrix);
    };
    // Clean up
    ~Cube() {
        // Delete data from OpenGL
        glDeleteBuffers(1, &ibo);
        glDeleteBuffers(1, &cbo);
        glDeleteBuffers(1, &vbo);
        glDeleteVertexArrays(1, &vao);
    }

    // Set the object transformation matrix
    void updateModelMatrix() {
        // The cube should orbit around the blue (Z) axis while also spinning around
        // its own local Z axis. We first rotate the local cube, then translate it to
        // its orbit position.
        modelMatrix = glm::mat4{1.0f};
        modelMatrix = glm::translate(modelMatrix, position);
        modelMatrix = glm::rotate(modelMatrix, rotation.z, glm::vec3{0.0f, 0.0f, 1.0f});
        modelMatrix = glm::rotate(modelMatrix, rotation.y, glm::vec3{0.0f, 1.0f, 0.0f});
        modelMatrix = glm::rotate(modelMatrix, rotation.x, glm::vec3{1.0f, 0.0f, 0.0f});
        modelMatrix = glm::scale(modelMatrix, scale);
    }

    void updateViewMatrix(glm::vec3 viewRotation) {
        // Camera is placed back along Z and tilted to see the origin from an angle.
        viewMatrix = glm::mat4{1.0f};
        viewMatrix = glm::translate(viewMatrix, glm::vec3{0.0f, 0.0f, -10.0f});
        viewMatrix = glm::rotate(viewMatrix, glm::radians(15.0f), glm::vec3{1.0f, 0.0f, 0.0f});
        viewMatrix = glm::rotate(viewMatrix, glm::radians(25.0f), glm::vec3{0.0f, 1.0f, 0.0f});
      }

    // Draw polygons
    void render(){
        // Update transformation and color uniforms in the shader
        program.use();
        program.setUniform("OverallColor", color);
        program.setUniform("ModelMatrix", modelMatrix);
        program.setUniform("ViewMatrix", viewMatrix);

        glBindVertexArray(vao);
        glDrawElements(GL_TRIANGLES, (GLsizei) indices.size() * 3, GL_UNSIGNED_INT, 0);
    };
};

class OriginWindow : public ppgso::Window {
private:
    Cube axisX, axisY, axisZ;
    Cube cube;

    glm::vec3 viewRotation{0,0,0};
    std::map<int, int> keyboard;
    float lastTime = 0.0f;
    float moveSpeed = 2.5f;

public:
    OriginWindow() : Window{"task5_3d_origin", SIZE, SIZE} {
        glfwSetInputMode(window, GLFW_STICKY_KEYS, 1);

        // TODO: Set axis colors to red,green and blue...and cube color to grey
        axisX.color = {1, 0, 0};  // red
        axisY.color = {0, 1, 0};  // green
        axisZ.color = {0, 0, 1};  // blue
        cube.color  = {0.6f, 0.6f, 0.6f}; // grey

        const float scaleMin = 0.03f;
        const float scaleMax = 10.00f;

        // TODO: Set axis scaling in X,Y,Z directions...hint use scaleMin in tangent directions and scaleMax in the axis direction
        axisX.scale = {scaleMax, scaleMin, scaleMin};
        axisY.scale = {scaleMin, scaleMax, scaleMin};
        axisZ.scale = {scaleMin, scaleMin, scaleMax};

        cube.scale = {1.0f, 1.0f, 1.0f};
        cube.position = {-2.0f, 0.0f, 5.0f};
    }

    void onKey(int key, int scanCode, int action, int mods) override {
        if (action == GLFW_PRESS || action == GLFW_REPEAT) {
            keyboard[key] = 1;
        } else if (action == GLFW_RELEASE) {
            keyboard[key] = 0;
        }

        if (key == GLFW_KEY_R && action == GLFW_PRESS) {
            cube.position = {-2.0f, 0.0f, 5.0f};
            cube.rotation = {0.0f, 0.0f, 0.0f};
        }
    }

    void onIdle() override {
        // Set gray background
        glClearColor(.1f,.1f,.1f,1.0f);
        // Clear depth and color buffers
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        float now = (float) glfwGetTime();
        float dt = now - lastTime;
        lastTime = now;

        // Keyboard movement of the cube
        if (keyboard[GLFW_KEY_LEFT] || keyboard[GLFW_KEY_A]) {
            cube.position.x -= moveSpeed * dt;
        }
        if (keyboard[GLFW_KEY_RIGHT] || keyboard[GLFW_KEY_D]) {
            cube.position.x += moveSpeed * dt;
        }
        if (keyboard[GLFW_KEY_UP] || keyboard[GLFW_KEY_W]) {
            cube.position.y += moveSpeed * dt;
        }
        if (keyboard[GLFW_KEY_DOWN] || keyboard[GLFW_KEY_S]) {
            cube.position.y -= moveSpeed * dt;
        }
        if (keyboard[GLFW_KEY_Q]) {
            cube.position.z -= moveSpeed * dt;
        }
        if (keyboard[GLFW_KEY_E]) {
            cube.position.z += moveSpeed * dt;
        }

        // The cube is placed at a radius from the origin and rotates around the blue Z axis.
        float t = now;
        cube.position.x = std::cos(t) * 2.5f;
        cube.position.y = std::sin(t) * 2.5f;
        cube.position.z = 0.0f;
        cube.rotation.z = t * 2.0f;

        cube.updateViewMatrix(viewRotation);
        axisX.updateViewMatrix(viewRotation);
        axisY.updateViewMatrix(viewRotation);
        axisZ.updateViewMatrix(viewRotation);

        cube.updateModelMatrix();
        axisX.updateModelMatrix();
        axisY.updateModelMatrix();
        axisZ.updateModelMatrix();

        cube.render();
        axisX.render();
        axisY.render();
        axisZ.render();
     }
};

int main() {
    // Create our window
    auto window = OriginWindow{};

    // Main execution loop
    while (window.pollEvents()) {}

    return EXIT_SUCCESS;
}
