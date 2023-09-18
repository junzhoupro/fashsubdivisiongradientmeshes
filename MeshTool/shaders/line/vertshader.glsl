#version 410
// Vertex shader

layout (location = 0) in vec2 coordsIn;

uniform vec2 scaling;
uniform vec2 displacement;

void main() {
  gl_Position = vec4(scaling * (displacement + coordsIn), 0, 1);
}
