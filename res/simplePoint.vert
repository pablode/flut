#version 430 core

layout (location = 0) in vec3 vertPos;
layout (location = 1) in vec3 vertColor;

out vec3 fragPos;
out vec3 fragColor;

uniform mat4 ModelViewProjMat;
uniform mat4 ModelViewMat;
uniform mat3 NormalMat;

void main() {
  fragColor = vertColor;
  fragPos = (ModelViewMat * vec4(vertPos, 1.0)).xyz;
  gl_Position = ModelViewProjMat * vec4(vertPos, 1.0);
}
