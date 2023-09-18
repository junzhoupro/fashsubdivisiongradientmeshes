#version 410
// Geometry shader

const float PI = 3.1415926;
const int N = 32;

layout(points) in;
layout(invocations = 32) in;

uniform vec2 scaling;
uniform float radius;

layout(line_strip, max_vertices = 3) out;

// Approximates a circle using N line pieces
void main() {
  // First boundary vertex
  float angle_1 = 2 * PI * gl_InvocationID / N;
  vec2 offset_1 = radius * scaling * vec2(cos(angle_1), sin(angle_1));
  gl_Position = gl_in[0].gl_Position + vec4(offset_1, 0.0, 0.0);
  EmitVertex();

  // Second boundary vertex
  float angle_2 = 2 * PI * (gl_InvocationID + 1) / N;
  vec2 offset_2 = radius * scaling * vec2(cos(angle_2), sin(angle_2));
  gl_Position = gl_in[0].gl_Position + vec4(offset_2, 0.0, 0.0);
  EmitVertex();

  EndPrimitive();
}
