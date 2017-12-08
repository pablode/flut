#version 430 core

in vec3 fragPos;
in vec3 fragColor;

out vec4 finalColor;

void main() {
  finalColor = vec4(fragColor, 1.0);
}
