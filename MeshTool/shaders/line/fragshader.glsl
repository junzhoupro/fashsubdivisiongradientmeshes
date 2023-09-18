#version 410
// Fragment shader

uniform vec3 color;

layout (location = 0) out vec4 colorOut;

void main() {
  colorOut = vec4(color, 1.0);
}
