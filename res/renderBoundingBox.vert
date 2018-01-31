#version 430 core

/// Pipeline stage 4 (2/3).
/// Simple Vertex Shader which executes on Bounding Box
/// vertices. These are transformed using the MVP and
/// therefore display an OBB.

layout (location = 0) uniform mat4 modelViewProjection;

in vec3 vertPos;

void main() {
  gl_Position = modelViewProjection * vec4(vertPos, 1.0);
}
