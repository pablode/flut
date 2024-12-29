const float FLOAT_MIN = 1.175494351e-38;
const float GRID_EPS = 0.000001;
const float MAX_DENSITY = 30.0;
const vec3 PARTICLE_COLOR = vec3(0.0, 0.0, 0.6);

struct Particle
{
  vec3 position;
  float density;
  vec3 velocity;
  float pressure;
};

layout(binding = 0, std430) restrict readonly buffer particleBuf
{
  Particle particles[];
};

layout (location = 0) uniform mat4 VP;
layout (location = 1) uniform mat4 V;
layout (location = 2) uniform mat4 P;
layout (location = 3) uniform vec3 gridSize;
layout (location = 4) uniform vec3 gridOrigin;
layout (location = 5) uniform ivec3 gridRes;
layout (location = 6) uniform uint particleCount;
layout (location = 7) uniform float pointRadius;
layout (location = 8) uniform int colorMode;

out vec3 color;
out vec3 centerPos;
out vec2 uv;

vec2 UVS[4] = { vec2(0,0), vec2(1,0), vec2(0,1), vec2(1,1) };
vec2 OFFSETS[4] = { vec2(-1,-1), vec2(-1,+1), vec2(+1,-1), vec2(+1,+1) };

void main()
{
  uint gid = gl_VertexID / 4;
  uint lid = gl_VertexID % 4;

  vec3 particlePos = particles[gid].position;

  if (colorMode == 0)
  {
    color = PARTICLE_COLOR;
  }
  else if (colorMode == 1)
  {
    vec3 velocity = abs(particles[gid].velocity);
    float w = max(max(FLOAT_MIN, velocity.x), max(velocity.y, velocity.z));
    bool invalid = any(isnan(velocity)) || any(isinf(velocity));
    color = mix(velocity / w, vec3(1.0, 0.0, 0.0), float(invalid));
  }
  else if (colorMode == 2)
  {
    vec3 velocity = particles[gid].velocity;
    float speed = length(velocity);
    color = vec3(speed, speed, 0.0);
  }
  else if (colorMode == 3)
  {
    float density = particles[gid].density;
    float norm = density / MAX_DENSITY;
    bool invalid = (density <= 0.0) || any(isnan(density)) || any(isinf(density));
    color = mix(vec3(0.0, norm, 0.0), vec3(1.0, 0.0, 0.0), float(invalid));
  }
  else if (colorMode == 4)
  {
    vec3 normPos = (particlePos - gridOrigin) / gridSize;
    ivec3 voxelCoord = ivec3(normPos * (1.0f - GRID_EPS) * gridRes);
    color = vec3(voxelCoord) / gridRes;
  }

  centerPos = (V * vec4(particlePos, 1.0)).xyz;
  uv = UVS[lid];

  vec3 wsCameraRight = vec3(V[0][0], V[1][0], V[2][0]);
  vec3 wsCameraUp = vec3(V[0][1], V[1][1], V[2][1]);
  vec3 wsCameraForward = vec3(V[0][2], V[1][2], V[2][2]);

  vec3 wsVertPos = particlePos + (wsCameraRight * OFFSETS[lid].x + wsCameraUp * OFFSETS[lid].y) * pointRadius;

  gl_Position = VP * vec4(wsVertPos, 1.0);
}
