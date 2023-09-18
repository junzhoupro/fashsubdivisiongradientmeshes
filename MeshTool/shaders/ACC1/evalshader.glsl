#version 410
// Tesselation evaluation shader

layout (quads, equal_spacing, ccw) in;

layout (location = 0) in vec2 coordsIn[];
layout (location = 1) in vec3 colorIn[];

layout (location = 0) out vec3 colorOut;

vec4 bezierBasis(float u) {
  return vec4(pow(1 - u, 3),
              3 * u * pow(1 - u, 2),
              3 * (1 - u) * pow(u, 2),
              pow(u, 3));
}

void main() {
  // Compute bezier basis
  vec4 bU = bezierBasis(gl_TessCoord.x);
  vec4 bV = bezierBasis(gl_TessCoord.y);

  // Set coordinates
  vec2 coordsOut = bV[0] * (bU[0] * coordsIn[0] + bU[1] * coordsIn[1] + bU[2] * coordsIn[2] + bU[3] * coordsIn[4])
                   + bV[1] * (bU[0] * coordsIn[14] + bU[1] * coordsIn[3] + bU[2] * coordsIn[7] + bU[3] * coordsIn[5])
                   + bV[2] * (bU[0] * coordsIn[13] + bU[1] * coordsIn[15] + bU[2] * coordsIn[11] + bU[3] * coordsIn[6])
                   + bV[3] * (bU[0] * coordsIn[12] + bU[1] * coordsIn[10] + bU[2] * coordsIn[9] + bU[3] * coordsIn[8]);
  gl_Position = vec4(coordsOut, 0, 1);

  // Set color
  colorOut = bV[0] * (bU[0] * colorIn[0] + bU[1] * colorIn[1] + bU[2] * colorIn[2] + bU[3] * colorIn[4])
             + bV[1] * (bU[0] * colorIn[14] + bU[1] * colorIn[3] + bU[2] * colorIn[7] + bU[3] * colorIn[5])
             + bV[2] * (bU[0] * colorIn[13] + bU[1] * colorIn[15] + bU[2] * colorIn[11] + bU[3] * colorIn[6])
             + bV[3] * (bU[0] * colorIn[12] + bU[1] * colorIn[10] + bU[2] * colorIn[9] + bU[3] * colorIn[8]);
  }
