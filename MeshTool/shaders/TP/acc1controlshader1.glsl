#version 410 core
// Tesselation control shader

layout (vertices = 16) out;

layout (location = 0) in vec2 coordsIn[];
layout (location = 1) in vec3 colorIn[];

uniform int tessLevel;

layout (location = 0) out vec2 coordsOut[];
layout (location = 1) out vec3 colorOut[];

void main() {
  if (gl_InvocationID == 0) {
    gl_TessLevelInner[0] = tessLevel;
    gl_TessLevelInner[1] = tessLevel;
    gl_TessLevelOuter[0] = 2 * tessLevel;
    gl_TessLevelOuter[1] = tessLevel;
    gl_TessLevelOuter[2] = tessLevel;
    gl_TessLevelOuter[3] = 2 * tessLevel;
  }

  coordsOut[gl_InvocationID] = coordsIn[gl_InvocationID];
  colorOut[gl_InvocationID] = colorIn[gl_InvocationID];
}
