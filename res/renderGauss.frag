#version 430 core

/// Pipeline stage 4 (2/3).
/// Apply a one-dimensional gauss function to
/// a given texture with a given resolution in
/// one direction. First, this shader is executed
/// horizontally. Then, a second pass performs
/// vertical blur on the previous result.
/// The precomputed kernel covers an 11x11 area (sigma 2.25).

layout(location = 0) uniform vec2 res;
layout(location = 1) uniform vec2 dir;
layout(location = 2) uniform sampler2D tex;

out vec3 finalColor;

void main(void) {
  vec2 tc = gl_FragCoord.xy / res;
  vec4 sum = vec4(0.0);
  float blurX = 1.0 / res.y;
  float blurY = 1.0 / res.x;
  sum += texture(tex, vec2(tc.x - 5.0 * blurX * dir.x, tc.y - 5.0 * blurY * dir.y)) * 0.015724;
  sum += texture(tex, vec2(tc.x - 4.0 * blurX * dir.x, tc.y - 4.0 * blurY * dir.y)) * 0.037704;
  sum += texture(tex, vec2(tc.x - 3.0 * blurX * dir.x, tc.y - 3.0 * blurY * dir.y)) * 0.074433;
  sum += texture(tex, vec2(tc.x - 2.0 * blurX * dir.x, tc.y - 2.0 * blurY * dir.y)) * 0.120988;
  sum += texture(tex, vec2(tc.x - 1.0 * blurX * dir.x, tc.y - 1.0 * blurY * dir.y)) * 0.161927;
  sum += texture(tex, vec2(tc.x, tc.y)) * 0.178448;
  sum += texture(tex, vec2(tc.x + 1.0 * blurX * dir.x, tc.y + 1.0 * blurY * dir.y)) * 0.161927;
  sum += texture(tex, vec2(tc.x + 2.0 * blurX * dir.x, tc.y + 2.0 * blurY * dir.y)) * 0.120988;
  sum += texture(tex, vec2(tc.x + 3.0 * blurX * dir.x, tc.y + 3.0 * blurY * dir.y)) * 0.074433;
  sum += texture(tex, vec2(tc.x + 4.0 * blurX * dir.x, tc.y + 4.0 * blurY * dir.y)) * 0.037704;
  sum += texture(tex, vec2(tc.x + 5.0 * blurX * dir.x, tc.y + 5.0 * blurY * dir.y)) * 0.015724;
  finalColor = sum.rgb;
}
