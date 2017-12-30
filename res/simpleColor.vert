#version 430 core

const float EPS = 0.001;
const float MAX_DENSITY = 1000.0;

layout (location = 0) in vec3 vertPos;
layout (location = 1) in vec3 vertColor;

layout (std430, binding = 0) buffer positionBuf2 { float position2[]; };
layout (std430, binding = 1) buffer densityBuf { float density[]; };
layout (std430, binding = 2) buffer colorBuf { float color[]; };

layout (location = 0) uniform mat4 modelViewProj;
layout (location = 1) uniform mat4 view;
layout (location = 2) uniform mat4 projection;
layout (location = 3) uniform vec3 gridLength;
layout (location = 4) uniform vec3 gridOrigin;
layout (location = 5) uniform uvec3 gridResolution;
layout (location = 6) uniform uint particleCount;
layout (location = 7) uniform float pointRadius;
layout (location = 8) uniform int colorMode;
layout (location = 9) uniform int shadingMode;

out vec3 fragPos;
out vec3 fragColor;

uint cellId(uint cellCount, float length, float pos) {
  return uint((cellCount * (1.0 - EPS) * (length - pos)) / length);
}

vec3 voxelColor(uint particleCount, uint p) {
  float posX = position2[p * 3 + 0];
  float posY = position2[p * 3 + 1];
  float posZ = position2[p * 3 + 2];
  uint xCell = cellId(gridResolution.x, gridLength.x, posX - gridOrigin.x);
  uint yCell = cellId(gridResolution.y, gridLength.y, posY - gridOrigin.y);
  uint zCell = cellId(gridResolution.z, gridLength.z, posZ - gridOrigin.z);
  return vec3(xCell, yCell, zCell) / float(gridResolution);
}

void main() {
  fragColor = vertColor;
  int p = gl_VertexID;

  // 0: Initial
  // 1: Density
  // 2: Uniform Grid
  if (colorMode == 0) {
    fragColor = vec3(color[p * 3 + 0], color[p * 3 + 1], color[p * 3 + 2]);
  } else if (colorMode == 1) {
    fragColor = vec3(density[p] / MAX_DENSITY, density[p] / MAX_DENSITY, 1.0);
    if (density[p] <= 0.0000001 || isinf(density[p]) || isnan(density[p])) fragColor = vec3(1.0, 0.0, 0.0);
  } else if (colorMode == 2) {
    fragColor = voxelColor(particleCount, p);
  }

  fragPos = (view * vec4(vertPos, 1.0)).xyz;
  float dist = length(fragPos);
  gl_PointSize = pointRadius / dist;
  gl_Position = modelViewProj * vec4(vertPos, 1.0);
}
