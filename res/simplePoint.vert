#version 430 core

layout (location = 0) in vec3 vertPos;
layout (location = 1) in vec3 vertColor;

layout (location = 0) uniform mat4 modelViewProj;
layout (location = 1) uniform mat4 modelView;

out vec3 fragPos;
out vec3 fragColor;

void main() {
  fragColor = vertColor;
  fragPos = (modelView * vec4(vertPos, 1.0)).xyz;
  gl_Position = modelViewProj * vec4(vertPos, 1.0);
}
