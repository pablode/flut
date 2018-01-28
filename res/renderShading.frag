#version 430 core

/// Pipeline stage 4 (4).
/// Read GBuffer values, reconstruct position
/// from depth and do simple Phong shading.

const vec3 lightPos = vec3(0.0, 1.0, 0.0);
const float ambientCoeff = 0.3;
const float shininess = 20.0;
const float maxDepth = 0.999999;

layout (location = 0) uniform sampler2D depthTex;
layout (location = 1) uniform sampler2D colorTex;
layout (location = 2) uniform sampler2D normalTex;
layout (location = 3) uniform uint width;
layout (location = 4) uniform uint height;
layout (location = 5) uniform mat4 invProjection;
layout (location = 6) uniform mat4 view;

out vec4 finalColor;

///
vec3 getEyePos(vec2 coord) {
  // Reconstruct position from depth
  float viewportDepth = texture(depthTex, coord).x;
  float ndcDepth = viewportDepth * 2.0 - 1.0;
  vec4 clipSpacePos = vec4(coord * 2.0 - vec2(1.0), ndcDepth, 1.0);
  vec4 eyeSpacePos = invProjection * clipSpacePos;
  return eyeSpacePos.xyz / eyeSpacePos.w;
}

///
void main() {

  // Retrieve values from GBuffer
  vec2 texelSize = 1.0 / vec2(width, height);
  vec2 coord = gl_FragCoord.xy * texelSize;
  float viewportDepth = texture(depthTex, coord).x;
  if (viewportDepth > maxDepth) {
    discard;
    return;
  }
  vec3 normal = texture(normalTex, coord).xyz;
  vec3 color = texture(colorTex, coord).xyz;

  // Reconstruct position from depth
  vec3 eyeSpacePos = getEyePos(coord);

  // Diffuse
  vec3 lightPosEye = (view * vec4(lightPos, 1.0)).xyz;
  vec3 lightDir = normalize(lightPosEye - eyeSpacePos.xyz);
  float dcoeff = max(0.0, dot(normal, lightDir));
  vec3 diffColor = color * dcoeff;

  // Specular
  vec3 incidence = normalize(lightPosEye - eyeSpacePos.xyz);
  vec3 reflection = reflect(-incidence, normal);
  vec3 surfaceToCamera = normalize(vec3(0.0) - eyeSpacePos.xyz);
  float cosAngle = max(0.0, dot(surfaceToCamera, reflection));
  float scoeff = pow(cosAngle, shininess);
  vec3 specColor = vec3(1.0) * scoeff;

  // Final
  vec3 ambientColor = ambientCoeff * color;
  finalColor = vec4(ambientColor + diffColor + specColor, 1.0);
}
