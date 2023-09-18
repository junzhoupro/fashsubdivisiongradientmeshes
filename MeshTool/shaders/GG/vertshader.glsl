#version 410
// Vertex shader

layout (location = 0) in vec2 coordsIn;
layout (location = 1) in vec3 colorIn;

layout (location = 0) out vec2 coordsOut;
layout (location = 1) out vec3 colorOut;
layout (location = 2) flat out int instanceOut;

void main() {
  coordsOut = coordsIn;
  colorOut = colorIn;
  instanceOut = gl_InstanceID;
}
