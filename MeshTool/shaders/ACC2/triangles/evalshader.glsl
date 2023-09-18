#version 410
// Tesselation evaluation shader

layout (triangles, equal_spacing, ccw) in;

layout (location = 0) in vec2 coordsIn[];
layout (location = 1) in vec3 colorIn[];

layout (location = 0) out vec3 colorOut;

vec2 interpolate_coords() {
  // Shortcut variables
  float u = gl_TessCoord.x;
  float v = gl_TessCoord.y;
  float w = gl_TessCoord.z;

  vec2 p0 = coordsIn[0];
  vec2 e0p = coordsIn[1];
  vec2 e1m = coordsIn[2];
  vec2 f0p = coordsIn[3];
  vec2 f1m = coordsIn[4];
  vec2 p1 = coordsIn[5];
  vec2 e1p = coordsIn[6];
  vec2 e2m = coordsIn[7];
  vec2 f1p = coordsIn[8];
  vec2 f2m = coordsIn[9];
  vec2 p2 = coordsIn[10];
  vec2 e2p = coordsIn[11];
  vec2 e0m = coordsIn[12];
  vec2 f2p = coordsIn[13];
  vec2 f0m = coordsIn[14];

  // Interior points
  vec2 F0 = (v + w == 0) ? f0m : (w * f0m + v * f0p) / (v + w);
  vec2 F1 = (w + u == 0) ? f1m : (u * f1m + w * f1p) / (w + u);
  vec2 F2 = (u + v == 0) ? f2m : (v * f2m + u * f2p) / (u + v);

  // Compute coordinate
  return (u * u * u) * p0 + (v * v * v) * p1 + (w * w * w) * p2
         + 3 * u * v * (u + v) * (u * e0p + v * e1m)
         + 3 * v * w * (v + w) * (v * e1p + w * e2m)
         + 3 * w * u * (w + u) * (w * e2p + u * e0m)
         + 12 * u * v * w * (u * F0 + v * F1 + w * F2);
}

vec3 interpolate_color() {
  // Shortcut variables
  float u = gl_TessCoord.x;
  float v = gl_TessCoord.y;
  float w = gl_TessCoord.z;

  vec3 p0 = colorIn[0];
  vec3 e0p = colorIn[1];
  vec3 e1m = colorIn[2];
  vec3 f0p = colorIn[3];
  vec3 f1m = colorIn[4];
  vec3 p1 = colorIn[5];
  vec3 e1p = colorIn[6];
  vec3 e2m = colorIn[7];
  vec3 f1p = colorIn[8];
  vec3 f2m = colorIn[9];
  vec3 p2 = colorIn[10];
  vec3 e2p = colorIn[11];
  vec3 e0m = colorIn[12];
  vec3 f2p = colorIn[13];
  vec3 f0m = colorIn[14];

  // Interior points
  vec3 F0 = (v + w == 0) ? f0m : (w * f0m + v * f0p) / (v + w);
  vec3 F1 = (w + u == 0) ? f1m : (u * f1m + w * f1p) / (w + u);
  vec3 F2 = (u + v == 0) ? f2m : (v * f2m + u * f2p) / (u + v);

  // Compute color
  return (u * u * u) * p0 + (v * v * v) * p1 + (w * w * w) * p2
         + 3 * u * v * (u + v) * (u * e0p + v * e1m)
         + 3 * v * w * (v + w) * (v * e1p + w * e2m)
         + 3 * w * u * (w + u) * (w * e2p + u * e0m)
         + 12 * u * v * w * (u * F0 + v * F1 + w * F2);
}

void main() {
  gl_Position = vec4(interpolate_coords(), 0, 1);
  colorOut = interpolate_color();
}

