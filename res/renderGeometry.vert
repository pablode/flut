#version 460 core

const float FLOAT_MIN = 1.175494351e-38;
const float GRID_EPS = 0.000001;
const float MAX_DENSITY = 50.0;

layout (location = 0) in vec3 vertPos;
layout (location = 1) in vec3 vertColor;

struct Particle
{
  vec3 position;
  float density;
  vec3 color;
  float pressure;
  vec3 velocity;
  float padding;
};

layout(binding = 0, std430) restrict readonly buffer particleBuf
{
  Particle particles[];
};

layout (location = 0) uniform mat4 MVP;
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

out vec3 fragPos;
out vec3 fragColor;

void main()
{
  const int p = gl_VertexID;

  if (colorMode == 0)
  {
    fragColor = vertColor;
  }
  else if (colorMode == 1)
  {
    const vec3 velocity = abs(particles[p].velocity);
    const float w = max(max(FLOAT_MIN, velocity.x), max(velocity.y, velocity.z));
    const bool invalid = any(isnan(velocity)) || any(isinf(velocity));
    fragColor = mix(velocity / w, vec3(1.0, 0.0, 0.0), float(invalid));
  }
  else if (colorMode == 2)
  {
    const vec3 velocity = particles[p].velocity;
    const float speed = length(velocity);
    fragColor = vec3(speed, speed, 0.0);
  }
  else if (colorMode == 3)
  {
    const float density = particles[p].density;
    const float norm = density / MAX_DENSITY;
    const bool invalid = (density <= 0.0) || any(isnan(density)) || any(isinf(density));
    fragColor = mix(vec3(0.0, norm, 0.0), vec3(1.0, 0.0, 0.0), float(invalid));
  }
  else if (colorMode == 4)
  {
    const vec3 particlePos = particles[p].position;
    const vec3 normPos = (particlePos - gridOrigin) / gridSize;
    const ivec3 voxelCoord = ivec3(normPos * (1.0f - GRID_EPS) * gridRes);
    fragColor = gridRes / vec3(voxelCoord);
  }

  fragPos = (view * vec4(vertPos, 1.0)).xyz;

  const float dist = max(length(fragPos), FLOAT_MIN);

  gl_PointSize = pointRadius * (pointScale / dist);
  gl_Position = MVP * vec4(vertPos, 1.0);
}
