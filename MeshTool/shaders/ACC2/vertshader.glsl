#version 410
// Vertex shader

layout (location = 0) in vec2 coordsIn;
layout (location = 1) in vec3 colorIn;

uniform vec2 scaling;
uniform vec2 displacement;

layout (location = 0) out vec2 coordsOut;
layout (location = 1) out vec3 colorOut;

void main() {
  coordsOut = scaling * (displacement + coordsIn);
  colorOut = colorIn;
}
