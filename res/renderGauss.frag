#version 430 core

/// Pipeline stage 4 (2/3).
/// Apply a one-dimensional gauss function to
/// a given texture with a given resolution in
/// one direction. First, this shader is executed
/// horizontally. Then, a second pass performs
/// vertical blur on the previous result.
/// The precomputed kernel covers an 9x9 area.

uniform vec2 res;
uniform vec2 dir;
uniform sampler2D tex;

in vec2 texCoords;

out vec4 finalColor;

void main(void) {
  vec2 tc = texCoords;
  vec4 sum = vec4(0.0);
  float blurX = 1.0 / res.y;
  float blurY = 1.0 / res.x;
  sum += texture(tex, vec2(tc.x - 4.0 * blurX * dir.x, tc.y - 4.0 * blurY * dir.y)) * 0.0162162162;
  sum += texture(tex, vec2(tc.x - 3.0 * blurX * dir.x, tc.y - 3.0 * blurY * dir.y)) * 0.0540540541;
  sum += texture(tex, vec2(tc.x - 2.0 * blurX * dir.x, tc.y - 2.0 * blurY * dir.y)) * 0.1216216216;
  sum += texture(tex, vec2(tc.x - 1.0 * blurX * dir.x, tc.y - 1.0 * blurY * dir.y)) * 0.1945945946;
  sum += texture(tex, vec2(tc.x, tc.y)) * 0.2270270270;
  sum += texture(tex, vec2(tc.x + 1.0 * blurX * dir.x, tc.y + 1.0 * blurY * dir.y)) * 0.1945945946;
  sum += texture(tex, vec2(tc.x + 2.0 * blurX * dir.x, tc.y + 2.0 * blurY * dir.y)) * 0.1216216216;
  sum += texture(tex, vec2(tc.x + 3.0 * blurX * dir.x, tc.y + 3.0 * blurY * dir.y)) * 0.0540540541;
  sum += texture(tex, vec2(tc.x + 4.0 * blurX * dir.x, tc.y + 4.0 * blurY * dir.y)) * 0.0162162162;
  finalColor = vec4(sum.rgb, 1.0);
}
