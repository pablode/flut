#extension GL_ARB_bindless_texture: require

const float Z_THRESHOLD = 0.0005;
const float SMOOTH_DT = 0.0005;

layout (location = 0) uniform mat4 MVP;
layout (location = 1, bindless_sampler) uniform sampler2D depthTex;
layout (location = 2) uniform mat4 projection;
layout (location = 3) uniform ivec2 res;

out float finalDepth;

void main(void)
{
  vec2 coords = gl_FragCoord.xy / res;
  vec2 dx = vec2(1.0 / res.x, 0.0);
  vec2 dy = vec2(0.0, 1.0 / res.y);

  float z = texture(depthTex, coords).x;
  float zRight = texture(depthTex, coords + dx).x;
  float zLeft = texture(depthTex, coords - dx).x;
  float zTop = texture(depthTex, coords + dy).x;
  float zBottom = texture(depthTex, coords - dy).x;
  float zTopRight = texture(depthTex, coords + dx + dy).x;
  float zBottomLeft = texture(depthTex, coords - dx - dy).x;
  float zBottomRight = texture(depthTex, coords + dx - dy).x;
  float zTopLeft = texture(depthTex, coords - dx + dy).x;

  // Gradients (first derivative)
  float dzdx = 0.5 * (zRight - zLeft);
  float dzdy = 0.5 * (zTop - zBottom);

  // (central difference for better results)
  float dzdxy = (zTopRight + zBottomLeft - zBottomRight - zTopLeft) * 0.25;

  // Equation (3)
  float Fx = -projection[0][0]; // 2n / (r-l)
  float Fy = -projection[1][1]; // 2n / (t-b)
  float Cx = 2.0 / (res.x * Fx);
  float Cy = 2.0 / (res.y * Fy);
  float Cy2 = Cy * Cy;
  float Cx2 = Cx * Cx;

  // Equation (5)
  float D = Cy2 * (dzdx * dzdx) + Cx2 * (dzdy * dzdy) + Cx2 * Cy2 * (z * z);

  float dzdx2 = zRight + zLeft - z * 2.0;
  float dzdy2 = zTop + zBottom - z * 2.0;
  float dDdx = 2.0 * Cy2 * dzdx * dzdx2 + 2.0 * Cx2 * dzdy * dzdxy + 2.0 * Cx2 * Cy2 * z * dzdx;
  float dDdy = 2.0 * Cy2 * dzdx * dzdxy + 2.0 * Cx2 * dzdy * dzdy2 + 2.0 * Cx2 * Cy2 * z * dzdy;

  // Mean Curvature (7)(8)(6)
  float Ex = 0.5 * dzdx * dDdx - dzdx2 * D;
  float Ey = 0.5 * dzdy * dDdy - dzdy2 * D;
  float H2 = (Cy * Ex + Cx * Ey) / pow(D, 1.5);

  // Discontinuity handling
  bool screenEdge = (gl_FragCoord.xy == ivec2(0.0)) || (gl_FragCoord.xy == (res - ivec2(1)));
  bool bgPixel = (zRight == 1.0 || zLeft == 1.0 || zTop == 1.0 || zBottom == 1.0);
  bool depthDifferenceTooLarge = abs(zRight - z) > Z_THRESHOLD || abs(zLeft - z) > Z_THRESHOLD ||
                                 abs(zTop - z) > Z_THRESHOLD || abs(zBottom - z) > Z_THRESHOLD;
  bool zeroOutCurvature = bgPixel || screenEdge || depthDifferenceTooLarge;

  finalDepth = z + float(!zeroOutCurvature) * (0.5 * H2) * SMOOTH_DT;
}
