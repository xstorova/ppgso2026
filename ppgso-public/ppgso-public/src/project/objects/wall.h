#ifndef PROJECT_WALL_H
#define PROJECT_WALL_H

#include "../core/object.h"
#include "../core/light.h"

class Wall : public Object {
public:
    // x1,z1 -> x2,z2 = dva rohy podlahy (pôdorys steny)
    // yBottom, yTop  = spodok a vrch steny (presná výška)
    // thickness      = hrúbka steny
    // transparency   = priehľadnosť materiálu (1.0 = nepriehľadné)
    Wall(float x1, float z1, float x2, float z2,
         float yBottom, float yTop, float thickness,
         float transparency = 1.0f);

    void render(const Scene &scene, float widthPx, float heightPx) override;
    void renderDepth(const ppgso::Shader &depthShader) override;

    Material material;

private:
    static std::unique_ptr<ppgso::Shader> shader;
    static std::unique_ptr<ppgso::Mesh> mesh;
    static std::unique_ptr<ppgso::Texture> texture;
    float transparency{1.0f};
};

#endif // PROJECT_WALL_H