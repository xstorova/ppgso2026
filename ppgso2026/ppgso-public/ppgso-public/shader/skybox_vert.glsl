// =================================================================================
// [2b] Vyuzitie techniky mapovania na kocku (cube mapping) na vytvorenie Sky-box-u
// Vertex shader: Smerove suradnice vrcholu kocky (TexCoords) sluzia ako 3D vektor
// pre vzorkovanie Cube Map textury. Odstranenim translacie z View matice
// je zarucene, ze Skybox je nekonecne vzdialeny a hybe sa spolu s kamerou.
// =================================================================================
#version 330 core
layout (location = 0) in vec3 Position;

out vec3 TexCoords;

uniform mat4 ProjectionMatrix;
uniform mat4 ViewMatrix;

void main() {
    TexCoords = Position;
    mat4 staticView = mat4(mat3(ViewMatrix));
    vec4 pos = ProjectionMatrix * staticView * vec4(Position, 1.0);
    gl_Position = pos.xyww; // Maximalna hlbka (z/w = 1.0)
}
