#version 430 core

/// Pipeline stage 4 (4).
/// Read GBuffer values, reconstruct position
/// from depth and do simple Phong shading.

const vec3 lightPosEye = vec3(-0.577, -0.577, -0.577);
const vec3 lightDirEye = normalize(-lightPosEye);
const float ambientCoeff = 0.3;
const float shininess = 10.0;
const float maxDepth = 0.999999;

layout (location = 0) uniform sampler2D depthTex;
layout (location = 1) uniform sampler2D colorTex;
layout (location = 2) uniform sampler2D normalTex;
layout (location = 3) uniform uint width;
layout (location = 4) uniform uint height;
layout (location = 5) uniform mat4 invProjection;

out vec4 finalColor;

void main() {

  // Retrieve values from GBuffer
  vec2 coord = gl_FragCoord.xy / vec2(width, height);
  float viewportDepth = texture(depthTex, coord).x;
  if (viewportDepth > maxDepth) {
    discard;
    return;
  }
  vec3 normal = texture(normalTex, coord).xyz;
  vec3 color = texture(colorTex, coord).xyz;

  // Reconstruct position from depth
  float ndcDepth = viewportDepth * 2.0 - 1.0;
  vec4 clipSpacePos = vec4(coord * 2.0 - vec2(1.0), ndcDepth, 1.0);
  vec4 eyeSpacePos = invProjection * clipSpacePos;
  eyeSpacePos.xyz /= eyeSpacePos.w;

  // Diffuse
  float dcoeff = max(0.0, dot(normal, lightDirEye));
  vec3 diffColor = color * dcoeff;

  // Specular
  vec3 eye = normalize(-eyeSpacePos.xyz);
  vec3 reflection = normalize(reflect(-lightDirEye, normal));
  float scoeff = pow(max(dot(reflection, eye), 0.0f), shininess);
  vec3 specColor = vec3(1.0) * scoeff;

  // Final
  vec3 ambientColor = ambientCoeff * color;
  finalColor = vec4(ambientColor + diffColor + specColor, 1.0);
}
