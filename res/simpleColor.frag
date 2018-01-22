#version 430 core

const vec3 lightPos = vec3(-0.577, -0.577, -0.577);
const vec3 lightDir = normalize(-lightPos);
const float shininess = 10.0;

in vec3 fragPos;
in vec3 fragColor;

out vec4 finalColor;

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

  if (shadingMode == 0) {
    finalColor = vec4(fragColor, 1.0);
    vec4 eyeSpacePos = vec4(fragPos, 1.0);
    vec4 clipSpacePos = projection * eyeSpacePos;
    float ndcDepth = clipSpacePos.z / clipSpacePos.w;
    float windowDepth = 0.5 * ndcDepth + 0.5;
    gl_FragDepth = windowDepth;
    return;

  } else {

    // Visibility
    vec3 N;
    N.xy = gl_PointCoord * 2.0 - 1.0;
    float r2 = dot(N.xy, N.xy);
    if (r2 > 1.0)
      discard; // Outside of circle
    N.z = sqrt(1.0 - r2);

    // Depth
    vec4 eyeSpacePos = vec4(fragPos + N * pointRadius, 1.0);
    vec4 clipSpacePos = projection * eyeSpacePos;
    float ndcDepth = clipSpacePos.z / clipSpacePos.w;
    float windowDepth = 0.5 * ndcDepth + 0.5;
    gl_FragDepth = windowDepth;

    // Diffuse
    float dcoeff = max(0.0, dot(N, lightDir));
    vec3 diffColor = fragColor * dcoeff;

    // Specular
    vec3 eye = normalize(-fragPos.xyz);
    vec3 reflection = normalize(reflect(-lightDir, N));
    float scoeff = pow(max(dot(reflection, eye), 0.0f), shininess);
    vec3 specColor = vec3(1.0) * scoeff;

    float ambientCoeff = 0.3;
    vec3 ambientColor = ambientCoeff * fragColor;
    finalColor = vec4(ambientColor + diffColor + specColor, 1.0);
  }
}
