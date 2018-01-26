#version 430 core

/// Pipeline stage 4 (2/3/4).
/// By using the vertex id, we render a fullscreen
/// triangle using glDrawArrays(GL_TRIANGLES, 0, 3)
/// without the need of an additional VBO/VAO.

void main() {
  vec2 pos = vec2((gl_VertexID << 1) & 2, gl_VertexID & 2);
  gl_Position = vec4(pos * 2.0 + -1.0, 0.0, 1.0);
}
