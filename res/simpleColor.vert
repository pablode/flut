#version 430 core

const float EPS = 0.001;
const float MAX_DENSITY = 50000.0;

layout (location = 0) in vec3 vertPos;
layout (location = 1) in vec3 vertColor;

layout (std430, binding = 0) buffer positionBuf2 { float position2[]; };
layout (std430, binding = 1) buffer densityBuf2 { float density2[]; };

layout (location = 0) uniform mat4 modelViewProj;
layout (location = 1) uniform mat4 modelView;
layout (location = 2) uniform vec3 gridLength;
layout (location = 3) uniform vec3 gridOrigin;
layout (location = 4) uniform uvec3 cellsPerAxis;
layout (location = 5) uniform uint particleCount;

layout (location = 6) uniform int color;

out vec3 fragPos;
out vec3 fragColor;

uint cellId(uint cellCount, float length, float pos) {
  return uint((cellCount * (1.0 - EPS) * (length - pos)) / length);
}

vec3 voxelColor(uint particleCount, uint p) {
  float posX = position2[p * 6 + 0];
  float posY = position2[p * 6 + 1];
  float posZ = position2[p * 6 + 2];
  uint xCell = cellId(cellsPerAxis.x, gridLength.x, posX - gridOrigin.x);
  uint yCell = cellId(cellsPerAxis.y, gridLength.y, posY - gridOrigin.y);
  uint zCell = cellId(cellsPerAxis.z, gridLength.z, posZ - gridOrigin.z);
  return vec3(xCell, yCell, zCell) / float(cellsPerAxis);
}

void main() {
  fragColor = vertColor;
  int p = gl_VertexID;

  // 0: Initial
  // 1: Density
  // 2: Uniform Grid
  if (color == 0) {
    fragColor = vec3(position2[p * 6 + 3], position2[p * 6 + 4], position2[p * 6 + 5]);
  } else if (color == 1) {
    fragColor = vec3(density2[p] / MAX_DENSITY, density2[p] / MAX_DENSITY, 1.0);
  } else if (color == 2) {
    fragColor = voxelColor(particleCount, p);
  }

  fragPos = (modelView * vec4(vertPos, 1.0)).xyz;
  gl_Position = modelViewProj * vec4(vertPos, 1.0);
}
