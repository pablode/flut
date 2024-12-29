#extension GL_ARB_bindless_texture: require

const float Z_THRESHOLD = 5.0;
const float SMOOTH_DT = 0.0005;

layout (location = 0) uniform mat4 MVP;
layout (location = 1, bindless_sampler) uniform sampler2D depthTex;
layout (location = 2) uniform mat4 projection;
layout (location = 3) uniform ivec2 res;

out float finalDepth;

float depthToEyeSpaceZ(float depth)
{
  const float ndc = 2.0 * depth - 1.0;
  return 2.0 * NEAR * FAR / (FAR + NEAR - ndc * (FAR - NEAR));
}

vec4 depthToEyeSpaceZ(vec4 depth)
{
  const vec4 ndc = 2.0 * depth - 1.0;
  return 2.0 * NEAR * FAR / (FAR + NEAR - ndc * (FAR - NEAR));
}

void main(void)
{
  const vec2 coords = gl_FragCoord.xy / res;
  const vec2 dx = vec2(1.0 / res.x, 0.0);
  const vec2 dy = vec2(0.0, 1.0 / res.y);

  const float z = texture(depthTex, coords).x;
  const float zRight = texture(depthTex, coords + dx).x;
  const float zLeft = texture(depthTex, coords - dx).x;
  const float zTop = texture(depthTex, coords + dy).x;
  const float zBottom = texture(depthTex, coords - dy).x;

  if (z == 1.0)
  {
    finalDepth = 1.0;
    return;
  }

  // Disallow large changes in depth
  const float eyeZ = depthToEyeSpaceZ(z);
  const vec4 neighborEyeZ = depthToEyeSpaceZ(vec4(zRight, zLeft, zTop, zBottom));
  const vec4 zDiff = abs(eyeZ - neighborEyeZ);

  if (any(greaterThan(zDiff, vec4(Z_THRESHOLD))))
  {
    finalDepth = z;
    return;
  }

  // Gradient (first derivative) with border handling
  const float dzdx = (gl_FragCoord.x <= 1 || gl_FragCoord.x >= res.x - 1 ||
                      zRight == 1.0 || zLeft == 1.0) ? 0.0 : 0.5 * (zRight - zLeft);
  const float dzdy = (gl_FragCoord.y <= 1 || gl_FragCoord.y >= res.y - 1 ||
                      zTop == 1.0 || zBottom == 1.0) ? 0.0 : 0.5 * (zTop - zBottom);

  // Diagonal neighbors
  const float zTopRight = texture(depthTex, coords + dx + dy).x;
  const float zBottomLeft = texture(depthTex, coords - dx - dy).x;
  const float zBottomRight = texture(depthTex, coords + dx - dy).x;
  const float zTopLeft = texture(depthTex, coords - dx + dy).x;

  // Use central difference (for better results)
  const float dzdxy = (zTopRight + zBottomLeft - zBottomRight - zTopLeft) * 0.25;

  // Equation (3)
  const float Fx = -projection[0][0]; // 2n / (r-l)
  const float Fy = -projection[1][1]; // 2n / (t-b)
  const float Cx = 2.0 / (res.x * Fx);
  const float Cy = 2.0 / (res.y * Fy);
  const float Cy2 = Cy * Cy;
  const float Cx2 = Cx * Cx;

  // Equation (5)
  const float D = Cy2 * (dzdx * dzdx) + Cx2 * (dzdy * dzdy) + Cx2 * Cy2 * (z * z);

  const float dzdx2 = zRight + zLeft - z * 2.0;
  const float dzdy2 = zTop + zBottom - z * 2.0;
  const float dDdx = 2.0 * Cy2 * dzdx * dzdx2 + 2.0 * Cx2 * dzdy * dzdxy + 2.0 * Cx2 * Cy2 * z * dzdx;
  const float dDdy = 2.0 * Cy2 * dzdx * dzdxy + 2.0 * Cx2 * dzdy * dzdy2 + 2.0 * Cx2 * Cy2 * z * dzdy;

  // Mean Curvature (7)(8)(6)
  const float Ex = 0.5 * dzdx * dDdx - dzdx2 * D;
  const float Ey = 0.5 * dzdy * dDdy - dzdy2 * D;
  const float H2 = (Cy * Ex + Cx * Ey) / pow(D, 3.0 / 2.0);

  finalDepth = z + (0.5 * H2) * SMOOTH_DT;
}
