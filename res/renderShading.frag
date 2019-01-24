#version 430 core

#extension ARB_bindless_texture : require

/// Pipeline stage 4 (3).
/// Read GBuffer values and do Blinn-Phong shading.

const vec3 lightPos = vec3(0.0, 1.0, 0.0);
const float ambientCoeff = 0.3;
const float shininess = 25.0;

layout (location = 0) uniform mat4 modelViewProjection;
layout (location = 1, bindless_sampler) uniform sampler2D depthTex;
layout (location = 2, bindless_sampler) uniform sampler2D colorTex;
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

void main() {
  // Retrieve values from GBuffer
  vec2 texelSize = 1.0 / vec2(width, height);
  vec2 coord = gl_FragCoord.xy * texelSize;
  float viewportDepth = texture(depthTex, coord).x;
  if (viewportDepth == 1.0) {
    discard;
    return;
  }

  // Reconstruct position from depth
  vec3 eyeSpacePos = getEyePos(coord);

  // Reconstruct normal from depth
  vec3 ddx = getEyePos(coord + vec2(texelSize.x, 0.0)) - eyeSpacePos;
  vec3 ddx2 = eyeSpacePos - getEyePos(coord + vec2(-texelSize.x, 0.0));
  if (abs(ddx.z) > abs(ddx2.z)) {
    ddx = ddx2;
  }
  vec3 ddy = getEyePos(coord + vec2(0.0, texelSize.y)) - eyeSpacePos;
  vec3 ddy2 = eyeSpacePos - getEyePos(coord + vec2(0.0, -texelSize.y));
  if (abs(ddy.z) > abs(ddy2.z)) {
    ddy = ddy2;
  }
  vec3 normal = normalize(cross(ddx, ddy));

  // Diffuse
  vec3 color = texture(colorTex, coord).xyz;
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
