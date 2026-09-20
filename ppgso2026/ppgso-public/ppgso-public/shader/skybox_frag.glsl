// =================================================================================
// [2b] Vyuzitie techniky mapovania na kocku (cube mapping) na vytvorenie Sky-box-u
// Fragment shader: Vzorkovanie OpenGL samplerCube pomocou 3D smeroveho vektora TexCoords
// =================================================================================
#version 330 core
in vec3 TexCoords;
out vec4 FragColor;

uniform samplerCube skybox;

void main() {
    FragColor = texture(skybox, TexCoords);
}
