#ifndef PROJECT_SCENE_H
#define PROJECT_SCENE_H

#include <list>
#include <map>
#include <memory>

#include "camera.h"
#include "object.h"

class Scene {
public:
  Scene();

  void update(float dt);
  void render(float width, float height);
  void handleKey(int key, int action);

  std::map<int, int> keyboard;
  std::list<std::unique_ptr<Object>> objects;
  Camera camera;
};

#endif // PROJECT_SCENE_H
