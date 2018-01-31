#version 430 core

/// Pipeline stage 4 (2).
/// Depth buffer smoothing using Curvature Flow. This pass is applied
/// several times on alternating FBOs. The smoothed depth values are
/// then used to reconstruct positions and normals.
/// For details, check "Screen Space Fluid Rendering with Curvature Flow"
/// by Van der Laan, Green and Sainz (NVIDIA).
/// Thanks to pysph and simpleflow.

// TODO: Depth is not linear! Convert range start/end to view space.
const float DEPTH_THRESHOLD = 0.001;
const float SMOOTH_DT = 0.0005;

layout (location = 0) uniform sampler2D depthTex;
layout (location = 1) uniform mat4 projection;
layout (location = 2) uniform ivec2 res;

out float finalDepth;

void main(void) {
  vec2 coords = gl_FragCoord.xy / res;
  float z = texture(depthTex, coords).x;

  // Handle screen borders and background
  if (gl_FragCoord.x <= 1 || gl_FragCoord.x >= res.x - 1 ||
      gl_FragCoord.y <= 1 || gl_FragCoord.y >= res.y - 1 ||
      z == 1.0) {
    finalDepth = z;
    return;
  }

  // Gradient step
  vec2 dx = vec2(1.0 / res.x, 0.0);
  vec2 dy = vec2(0.0, 1.0 / res.y);

  // Direct neighbors (dz/dx)
  float right = texture(depthTex, coords + dx).x;
  float left = texture(depthTex, coords - dx).x;
  float top = texture(depthTex, coords + dy).x;
  float bottom = texture(depthTex, coords - dy).x;

  // Disallow large change in depth
  if (abs(z - right) > DEPTH_THRESHOLD || abs(z - left) > DEPTH_THRESHOLD ||
    abs(z - top) > DEPTH_THRESHOLD || abs(z - bottom) > DEPTH_THRESHOLD) {
    finalDepth = z;
    return;
  }

  // Gradient (first derivative)
  float dzdx = (right == 0.0 || left == 0.0) ? 0.0 : 0.5 * (right - left);
  float dzdy = (top == 0.0 || bottom == 0.0) ? 0.0 : 0.5 * (top - bottom);

  // Diagonal neighbors
  float topRight = texture(depthTex, coords + dx + dy).x;
  float bottomLeft = texture(depthTex, coords - dx - dy).x;
  float bottomRight = texture(depthTex, coords + dx - dy).x;
  float topLeft = texture(depthTex, coords - dx + dy).x;

  // Use central difference (for better results)
  float dzdxy = (+topRight + bottomLeft - bottomRight - topLeft) * 0.25;

  // Equation (3)
  float Fx = -projection[0][0]; // 2n / (r-l)
  float Fy = -projection[1][1]; // 2n / (t-b)
  float Cx = 2.0 / (res.x * Fx);
  float Cy = 2.0 / (res.y * Fy);
  float Cy2 = Cy * Cy;
  float Cx2 = Cx * Cx;

  // Equation (5)
  float D = Cy2 * (dzdx * dzdx) + Cx2 * (dzdy * dzdy) + Cx2 * Cy2 * (z * z);

  float dzdx2 = right + left - z * 2.0;
  float dzdy2 = top + bottom - z * 2.0;
  float dDdx = 2.0 * Cy2 * dzdx * dzdx2 + 2.0 * Cx2 * dzdy * dzdxy + 2.0 * Cx2 * Cy2 * z * dzdx;
  float dDdy = 2.0 * Cy2 * dzdx * dzdxy + 2.0 * Cx2 * dzdy * dzdy2 + 2.0 * Cx2 * Cy2 * z * dzdy;

  // Mean Curvature (7)(8)(6)
  float Ex = 0.5 * dzdx * dDdx - dzdx2 * D;
  float Ey = 0.5 * dzdy * dDdy - dzdy2 * D;
  float H2 = (Cy * Ex + Cx * Ey) / pow(D, 3.0 / 2.0);
  finalDepth = z + (0.5 * H2) * SMOOTH_DT;
}
