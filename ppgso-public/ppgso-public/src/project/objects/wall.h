#ifndef PROJECT_WALL_H
#define PROJECT_WALL_H

#include "../core/object.h"

class Wall : public Object {
public:
    // x1,z1 -> x2,z2 = dva rohy podlahy (pôdorys steny)
    // yBottom, yTop  = spodok a vrch steny (presná výška)
    // thickness      = hrúbka steny
    Wall(float x1, float z1, float x2, float z2,
         float yBottom, float yTop, float thickness);

    void render(const Camera &camera, float widthPx, float heightPx) override;

private:
    static std::unique_ptr<ppgso::Shader> shader;
    static std::unique_ptr<ppgso::Mesh> mesh;
    static std::unique_ptr<ppgso::Texture> texture;
};

#endif // PROJECT_WALL_H