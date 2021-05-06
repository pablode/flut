in vec3 color;
in vec3 centerPos;
in vec2 uv;

out vec3 finalColor;

layout (location = 0) uniform mat4 VP;
layout (location = 1) uniform mat4 V;
layout (location = 2) uniform mat4 P;
layout (location = 3) uniform vec3 gridSize;
layout (location = 4) uniform vec3 gridOrigin;
layout (location = 5) uniform ivec3 gridRes;
layout (location = 6) uniform uint particleCount;
layout (location = 7) uniform float pointRadius;
layout (location = 8) uniform int colorMode;

#define SPHERE
#define DEPTH_REPLACEMENT

void main(void)
{
  finalColor = color;

#ifdef SPHERE
    vec3 N;

    N.xy = uv * 2.0 - 1.0;

    float r2 = dot(N.xy, N.xy);

    if (r2 > 1.0)
        discard;

#ifdef DEPTH_REPLACEMENT
    N.z = sqrt(1.0 - r2);

    vec4 pos_eye_space = vec4(centerPos + N * pointRadius, 1.0);
    vec4 pos_clip_space = P * pos_eye_space;
    float depth_ndc = pos_clip_space.z / pos_clip_space.w;
    float depth_winspace = depth_ndc * 0.5 + 0.5;
    gl_FragDepth = depth_winspace;
#endif
#endif
}
