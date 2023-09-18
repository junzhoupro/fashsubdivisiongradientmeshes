#version 410
// Tesselation control shader

layout (vertices = 20) out;

layout (location = 0) in vec2 coordsOut[];
layout (location = 1) in vec3 colorOut[];

uniform int tessLevel;

layout (location = 0) out vec2 coordsIn[];
layout (location = 1) out vec3 colorIn[];

void main() {
  if (gl_InvocationID == 0) {
    gl_TessLevelInner[0] = tessLevel;
    gl_TessLevelInner[1] = tessLevel;
    gl_TessLevelOuter[0] = tessLevel;
    gl_TessLevelOuter[1] = tessLevel;
    gl_TessLevelOuter[2] = tessLevel;
    gl_TessLevelOuter[3] = tessLevel;
  }

  coordsIn[gl_InvocationID] = coordsOut[gl_InvocationID];
  colorIn[gl_InvocationID] = colorOut[gl_InvocationID];
}
