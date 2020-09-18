#version 430 core

/// Pipeline stage 4 (1).
/// Output particle color.

in vec3 fragPos;
in vec3 fragColor;

out vec4 finalColor;

layout (location = 0) uniform mat4 modelViewProj;
layout (location = 1) uniform mat4 view;
layout (location = 2) uniform mat4 projection;
layout (location = 3) uniform vec3 gridSize;
layout (location = 4) uniform vec3 gridOrigin;
layout (location = 5) uniform ivec3 gridRes;
layout (location = 6) uniform uint particleCount;
layout (location = 7) uniform float pointRadius;
layout (location = 8) uniform float pointScale;
layout (location = 9) uniform int colorMode;
layout (location = 10) uniform int shadingMode;

void main(){
  finalColor = vec4(fragColor, 1.0);
}
