in vec3 fragPos;
in vec3 fragColor;

out vec3 finalColor;

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

void main()
{
  vec4 eyeSpacePos;

  if (shadingMode == 0)
  {
    eyeSpacePos = vec4(fragPos, 1.0);
  }
  else
  {
    vec3 N;

    N.xy = gl_PointCoord * vec2(2.0, -2.0) + vec2(-1.0, 1.0);

    const float r2 = dot(N.xy, N.xy);

    if (r2 > 1.0)
    {
      // Outside of circle
      discard;
    }

    N.z = sqrt(1.0 - r2);

    eyeSpacePos = vec4(fragPos + N * pointRadius, 1.0);
  }

  const vec4 clipSpacePos = projection * eyeSpacePos;
  const float ndcDepth = clipSpacePos.z / clipSpacePos.w;
  const float windowDepth = 0.5 * ndcDepth + 0.5;

  gl_FragDepth = windowDepth;

  finalColor = fragColor;
}
