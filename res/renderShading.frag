#version 430 core

/// Pipeline stage 4 (4).
/// Read GBuffer values and do Blinn-Phong shading.

const vec3 lightPos = vec3(0.0, 1.0, 0.0);
const float ambientCoeff = 0.3;
const float shininess = 25.0;

layout (location = 0) uniform sampler2D positionTex;
layout (location = 1) uniform sampler2D colorTex;
layout (location = 2) uniform sampler2D normalTex;
layout (location = 3) uniform uint width;
layout (location = 4) uniform uint height;
layout (location = 5) uniform mat4 invProjection;
layout (location = 6) uniform mat4 view;

out vec4 finalColor;

void main() {
  // Retrieve values from GBuffer
  vec2 texelSize = 1.0 / vec2(width, height);
  vec2 coord = gl_FragCoord.xy * texelSize;
  vec3 eyeSpacePos = texture(positionTex, coord).xyz;
  if (eyeSpacePos.z == 0.0f) {
    discard;
    return;
  }
  vec3 color = texture(colorTex, coord).xyz;
  vec3 normal = texture(normalTex, coord).xyz;
  normal = normalize(normal);

  // Diffuse
  vec3 lightPosEye = (view * vec4(lightPos, 1.0)).xyz;
  vec3 lightDir = normalize(lightPosEye - eyeSpacePos.xyz);
  float dcoeff = max(0.0, dot(normal, lightDir));
  vec3 diffColor = color * dcoeff;

  // Specular (Blinn-Phong)
  vec3 incidence = normalize(lightPosEye - eyeSpacePos.xyz);
  vec3 viewDir = normalize(vec3(0.0) - eyeSpacePos.xyz);
  vec3 halfDir = normalize(incidence + viewDir);
  float cosAngle = max(dot(halfDir, normal), 0.0);
  float scoeff = pow(cosAngle, shininess);
  vec3 specColor = vec3(1.0) * scoeff;

  // Final
  vec3 ambientColor = ambientCoeff * color;
  finalColor = vec4(ambientColor + diffColor + specColor, 1.0);
}
