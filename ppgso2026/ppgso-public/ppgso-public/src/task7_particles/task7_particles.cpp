// Task 7 - Dynamically generate objects in a 3D scene
//        - Implement a particle system where particles have position and speed
//        - Any object can be a generator and can add objects to the scene
//        - Create dynamic effect such as fireworks, rain etc.
//        - Encapsulate camera in a class

#include <iostream>
#include <vector>
#include <map>
#include <list>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/euler_angles.hpp>
#include <glm/gtx/transform.hpp>

#include <ppgso/ppgso.h>

#include <shaders/color_vert_glsl.h>
#include <shaders/color_frag_glsl.h>

const unsigned int SIZE = 512;

class Camera {
public:
  // TODO: Add parameters
  glm::vec3 position{-3, 2, 8};
  glm::vec3 target{0, 0, 0};
  glm::vec3 up{0, 1, 0};

  glm::mat4 viewMatrix;
  glm::mat4 projectionMatrix;

  /// Representaiton of
  /// \param fov - Field of view (in degrees)
  /// \param ratio - Viewport ratio (width/height)
  /// \param near - Distance of the near clipping plane
  /// \param far - Distance of the far clipping plane
  Camera(float fov = 45.0f, float ratio = 1.0f, float near = 0.1f, float far = 10.0f) {
    // TODO: Initialize perspective projection (hint: glm::perspective)
    projectionMatrix = glm::perspective(glm::radians(fov), ratio, near, far);
    update();
  }

  /// Recalculate viewMatrix from position, rotation and scale
  void update() {
    // TODO: Update viewMatrix (hint: glm::lookAt)
    viewMatrix = glm::lookAt(position, target, up);
  }
};

/// Abstract renderable object interface
class Renderable; // Forward declaration for Scene
using Scene = std::list<std::unique_ptr<Renderable>>; // Type alias

class Renderable {
public:
  // Virtual destructor is needed for abstract interfaces
  virtual ~Renderable() = default;

  /// Render the object
  /// \param camera - Camera to use for rendering
  virtual void render(const Camera& camera) = 0;

  /// Update the object. Useful for specifing animation and behaviour.
  /// \param dTime - Time delta
  /// \param scene - Scene reference
  /// \return - Return true to keep object in scene
  virtual bool update(float dTime, Scene &scene) = 0;
};

/// Basic particle that will render a sphere
/// TODO: Implement Renderable particle
class Particle final : public Renderable {
  // Static resources shared between all particles
  static std::unique_ptr<ppgso::Mesh> mesh;
  static std::unique_ptr<ppgso::Shader> shader;

  glm::vec3 position;
  glm::vec3 speed;
  glm::vec3 color;
  float age = 0.0f;
  float lifespan = 20.0f;

  // TODO: add more parameters as needed
public:
  /// Construct a new Particle
  /// \param p - Initial position
  /// \param s - Initial speed
  /// \param c - Color of particle
  Particle(glm::vec3 p, glm::vec3 s, glm::vec3 c)
  : position(p), speed(s), color(c), age(0.0f), lifespan(20.0f)
  {
    // First particle will initialize resources
    if (!shader) shader = std::make_unique<ppgso::Shader>(color_vert_glsl, color_frag_glsl);
    if (!mesh) mesh = std::make_unique<ppgso::Mesh>("sphere.obj");
  }

  bool update(float dTime, Scene &scene) override {
    // TODO: Animate position using speed and dTime.
    // - Return true to keep the object alive
    // - Returning false removes the object from the scene
    // - hint: you can add more particles to the scene here also

    age += dTime;
    if (age > lifespan)
      return false;

    if (scene.size() > 5000) return false;

    position += speed * dTime;


    speed.y -= 1.5f * dTime;

    float floorY = 0.0f;
    float floorSize = 2.5f;

    bool aboveFloor =
        position.x > -floorSize && position.x < floorSize &&
        position.z > -floorSize && position.z < floorSize;

    // Ak je častica nad podlahou, odrazí sa
    if (aboveFloor && position.y < floorY) {
      position.y = floorY;
      speed.y = -speed.y * 0.4f; // odraz s útlmom
      speed.x *= 0.8f;
      speed.z *= 0.8f;
    }

    // Ak častica spadne mimo podlahu, nech padá ďalej dole
    if (!aboveFloor && position.y < -5.0f) {
      return false; // zmizne zo scény po páde mimo podlahu
    }

    if (age < 0.1f && rand() % 10 < 2) {
      glm::vec3 s{
        ((rand() % 100) / 100.0f - 0.5f) * 2.0f,
        ((rand() % 100) / 100.0f) * 3.0f,
        ((rand() % 100) / 100.0f - 0.5f) * 2.0f
      };
      glm::vec3 c{1.0f, 0.5f + (rand() % 50) / 100.0f, 0.0f};
      scene.push_back(std::make_unique<Particle>(position, s, c));
    }

    return true;
  }

  void render(const Camera& camera) override {
    // TODO: Render the object
    // - Use the shader
    // - Setup all needed shader inputs
    // - hint: use OverallColor in the color_vert_glsl shader for color
    // - Render the mesh

    shader->use();
    shader->setUniform("ProjectionMatrix", camera.projectionMatrix);
    shader->setUniform("ViewMatrix", camera.viewMatrix);

    glm::mat4 modelMatrix = glm::translate(position) * glm::scale(glm::vec3(0.1f));
    shader->setUniform("ModelMatrix", modelMatrix);
    shader->setUniform("OverallColor", color);

    mesh->render();
  }
};

class Emitter final : public Renderable {
  glm::vec3 position;
  float cooldown = 0.0f;

public:
  Emitter(glm::vec3 pos) : position(pos) {}

  bool update(float dTime, Scene &scene) override {
    cooldown -= dTime;

    // Každých 0.05 sekundy vytvorí novú časticu
    if (cooldown <= 0.0f) {
      cooldown = 0.05f;

      glm::vec3 p = position;
      glm::vec3 s{
        ((rand() % 100) / 100.0f - 0.5f) * 1.0f,
        -((rand() % 100) / 100.0f) * 2.5f, // smerom nadol
        ((rand() % 100) / 100.0f - 0.5f) * 1.0f
      };
      glm::vec3 c{1.0f, (rand() % 100) / 200.0f + 0.5f, 0.0f};
      scene.push_back(std::make_unique<Particle>(p, s, c));
    }

    return true;
  }

  void render(const Camera& camera) override {
    static auto shader = std::make_unique<ppgso::Shader>(color_vert_glsl, color_frag_glsl);
    static auto mesh = std::make_unique<ppgso::Mesh>("sphere.obj");

    shader->use();
    shader->setUniform("ProjectionMatrix", camera.projectionMatrix);
    shader->setUniform("ViewMatrix", camera.viewMatrix);

    glm::mat4 modelMatrix = glm::translate(position) * glm::scale(glm::vec3(0.2f));
    shader->setUniform("ModelMatrix", modelMatrix);
    shader->setUniform("OverallColor", glm::vec3(0,1,0)); // zelený emitter

    mesh->render();
  }
};

class Floor final : public Renderable {
public:
  bool update(float dTime, Scene &scene) override { return true; }

  void render(const Camera& camera) override {
    static auto shader = std::make_unique<ppgso::Shader>(color_vert_glsl, color_frag_glsl);
    static auto mesh = std::make_unique<ppgso::Mesh>("cube.obj");

    shader->use();
    shader->setUniform("ProjectionMatrix", camera.projectionMatrix);
    shader->setUniform("ViewMatrix", camera.viewMatrix);

    glm::mat4 modelMatrix = glm::translate(glm::vec3(0, -0.05f, 0)) * glm::scale(glm::vec3(5, 0.1f, 5));
    shader->setUniform("ModelMatrix", modelMatrix);
    shader->setUniform("OverallColor", glm::vec3(0.3f, 0.3f, 0.3f));

    mesh->render();
  }
};

// Static resources need to be instantiated outside of the class as they are globals
std::unique_ptr<ppgso::Mesh> Particle::mesh;
std::unique_ptr<ppgso::Shader> Particle::shader;

class ParticleWindow : public ppgso::Window {
private:
  // Scene of objects
  Scene scene;

  // Create camera
  Camera camera = {120.0f, (float)width/(float)height, 1.0f, 400.0f};

  // Store keyboard state
  std::map<int, int> keys;
public:
  ParticleWindow() : Window{"task7_particles", SIZE, SIZE} {
    // Initialize OpenGL state
    // Enable Z-buffer
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);

    scene.push_back(std::make_unique<Emitter>(glm::vec3{0,5,0}));
    scene.push_back(std::make_unique<Floor>());
  }

  void onKey(int key, int scanCode, int action, int mods) override {
    // Collect key state in a map
    keys[key] = action;
    if (keys[GLFW_KEY_SPACE]) {
      // TODO: Add renderable object to the scene
      for (int i = 0; i < 40; i++)
      {
        glm::vec3 p{0, 0, 0};
        glm::vec3 s{
          ((rand() % 100) / 100.0f - 0.5f) * 6.0f,
          ((rand() % 100) / 100.0f) * 8.0f,
          ((rand() % 100) / 100.0f - 0.5f) * 6.0f
        };
        glm::vec3 c{(rand()%100)/100.0f, (rand()%100)/100.0f, (rand()%100)/100.0f};
        scene.push_back(std::make_unique<Particle>(p, s, c));
      }
    }
  }

  void onIdle() override {
    // Track time
    static auto time = (float) glfwGetTime();
    // Compute time delta
    float dTime = (float)glfwGetTime() - time;
    time = (float) glfwGetTime();

    // Set gray background
    glClearColor(.1f,.1f,.1f,1.0f);

    // Clear depth and color buffers
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Update all objects in scene
    // Because we need to delete while iterating this is implemented using c++ iterators
    // In most languages mutating the container during iteration is undefined behaviour
    auto i = std::begin(scene);
    while (i != std::end(scene)) {
      // Update object and remove from list if needed
      auto obj = i->get();
      if (!obj->update(dTime, scene))
        i = scene.erase(i);
      else
        ++i;
    }

    // Render every object in scene
    for(auto& object : scene) {
      object->render(camera);
    }
  }
};

int main() {
  // Create new window
  auto window = ParticleWindow{};

  // Main execution loop
  while (window.pollEvents()) {}

  return EXIT_SUCCESS;
}
