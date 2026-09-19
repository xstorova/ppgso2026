#version 330 core
uniform vec3 OverallColor;
out vec4 FragmentColor;

void main() {
    FragmentColor = vec4(OverallColor, 1.0);
}
