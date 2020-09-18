#version 460 core

layout (location = 0) uniform mat4 MVP;

in vec3 vertPos;

void main()
{
  gl_Position = MVP * vec4(vertPos, 1.0);
}
