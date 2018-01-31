#version 430 core

/// Pipeline stage 4 (1).
/// Calculate particle sphere depth, normal and color.

in vec3 fragPos;
in vec3 fragColor;

out vec3 finalColor;

layout (location = 0) uniform mat4 modelViewProj;
layout (location = 1) uniform mat4 view;
layout (location = 2) uniform mat4 projection;
layout (location = 3) uniform vec3 gridLength;
layout (location = 4) uniform vec3 gridOrigin;
layout (location = 5) uniform uvec3 gridResolution;
layout (location = 6) uniform uint particleCount;
layout (location = 7) uniform float pointRadius;
layout (location = 8) uniform float pointScale;
layout (location = 9) uniform int colorMode;
layout (location = 10) uniform int shadingMode;

void main() {
  vec4 eyeSpacePos;
  vec3 N;

  if (shadingMode == 0) {
    // Flat points
    N = vec3(0.0, 0.0, 1.0);
    eyeSpacePos = vec4(fragPos, 1.0);

  } else {
    // Visibility & Normal
    N.xy = gl_PointCoord * vec2(2.0, -2.0) + vec2(-1.0, 1.0);
    float r2 = dot(N.xy, N.xy);
    if (r2 > 1.0)
      discard; // Outside of circle
    N.z = sqrt(1.0 - r2);

    // Position
    eyeSpacePos = vec4(fragPos + N * pointRadius, 1.0);
  }

  // Depth
  vec4 clipSpacePos = projection * eyeSpacePos;
  float ndcDepth = clipSpacePos.z / clipSpacePos.w;
  float windowDepth = 0.5 * ndcDepth + 0.5;
  gl_FragDepth = windowDepth;

  // Output
  finalColor = fragColor;
}
