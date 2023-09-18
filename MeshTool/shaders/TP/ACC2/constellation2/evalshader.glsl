#version 410
// Tesselation evaluation shader

layout (triangles, equal_spacing, ccw) in;

layout (location = 0) in vec2 coordsIn[];
layout (location = 1) in vec3 colorIn[];
patch in int instance;

layout (location = 0) out vec3 colorOut;

vec4 bezierBasis(float u) {
  return vec4(pow(1 - u, 3),
              3 * u * pow(1 - u, 2),
              3 * (1 - u) * pow(u, 2),
              pow(u, 3));
}

vec2 getQuadCoords() {
  // EXPLANATION MAPPING
  //
  // b__________d
  // |\\____  3 |
  // | \    \___|
  // |  \  1  _/|
  // | 0 \  _/ 2|
  // a____\/____c
  //
  // - digits represent faces (that correspond to the instance id)
  // - chars represent corresponding triangle origins (where u direction is closest to horizontal)

  if (instance == 0)
    return vec2(.5 * gl_TessCoord.x, gl_TessCoord.y);
  if (instance == 1)
    return vec2(gl_TessCoord.x + .5 * gl_TessCoord.y, 1 - gl_TessCoord.y - .5 * gl_TessCoord.x);
  if (instance == 2)
    return vec2(1 - .5 * gl_TessCoord.x, .5 * gl_TessCoord.y);
  if (instance == 3)
    return vec2(1 - gl_TessCoord.x, 1 - .5 * gl_TessCoord.y);
}

vec2 interpolateCoords(float u, float v) {
  // Construct G
  vec2 G[16];
  G[0]  = coordsIn[0];                                                                                        // p0
  G[1]  = coordsIn[1];                                                                                        // e0p
  G[2]  = coordsIn[2];                                                                                        // e1m
  G[3]  = coordsIn[5];                                                                                        // p1
  G[4]  = coordsIn[17];                                                                                       // e0m
  G[5]  = (u + v == 0) ? coordsIn[3]  : (u * coordsIn[3] + v * coordsIn[19]) / (u + v);                     // F0
  G[6]  = (1 - u + v == 0) ? coordsIn[8]  : ((1 - u) * coordsIn[4] + v * coordsIn[8]) / (1 - u + v);        // F1
  G[7]  = coordsIn[6];                                                                                        // e1p
  G[8]  = coordsIn[16];                                                                                       // e3p
  G[9]  = (1 + u - v == 0) ? coordsIn[18] : (u * coordsIn[14] + (1 - v) * coordsIn[18]) / (1 + u - v);      // F3
  G[10] = (2 - u - v == 0) ? coordsIn[13] : ((1 - u) * coordsIn[13] + (1 - v) * coordsIn[9]) / (2 - u - v); // F2
  G[11] = coordsIn[7];                                                                                        // e2m
  G[12] = coordsIn[15];                                                                                       // p3
  G[13] = coordsIn[12];                                                                                       // e3m
  G[14] = coordsIn[11];                                                                                       // e2p
  G[15] = coordsIn[10];                                                                                       // p2

  // Compute B^3(u) and B^3(v)
  vec4 B3u = bezierBasis(u);
  vec4 B3v = bezierBasis(v);

  // Compute B^3(u) * G
  vec2 B3uG[4] = vec2[4](vec2(0), vec2(0), vec2(0), vec2(0));
  for (int i=0; i < 4; ++i)
    for (int j=0; j < 4; ++j)
      B3uG[i] += B3u[j] * G[4 * i + j];

  // Compute B^3(u) * G * B^3(v)
  vec2 coords = vec2(0);
  for (int i = 0; i < 4; ++i)
    coords += B3uG[i] * B3v[i];

  return coords;
}

vec3 interpolateColor(float u, float v) {
  // Construct G
  vec3 G[16];
  G[0]  = colorIn[0];                                                                                      // p0
  G[1]  = colorIn[1];                                                                                      // e0p
  G[2]  = colorIn[2];                                                                                      // e1m
  G[3]  = colorIn[5];                                                                                      // p1
  G[4]  = colorIn[17];                                                                                     // e0m
  G[5]  = (u + v == 0) ? colorIn[3]  : (u * colorIn[3] + v * colorIn[19]) / (u + v);                       // F0
  G[6]  = (1 - u + v == 0) ? colorIn[8]  : ((1 - u) * colorIn[4] + v * colorIn[8]) / (1 - u + v);          // F1
  G[7]  = colorIn[6];                                                                                      // e1p
  G[8]  = colorIn[16];                                                                                     // e3p
  G[9]  = (1 + u - v == 0) ? colorIn[18] : (u * colorIn[14] + (1 - v) * colorIn[18]) / (1 + u - v);        // F3
  G[10] = (2 - u - v == 0) ? colorIn[13] : ((1 - u) * colorIn[13] + (1 - v) * colorIn[9]) / (2 - u - v);   // F2
  G[11] = colorIn[7];                                                                                      // e2m
  G[12] = colorIn[15];                                                                                     // p3
  G[13] = colorIn[12];                                                                                     // e3m
  G[14] = colorIn[11];                                                                                     // e2p
  G[15] = colorIn[10];                                                                                     // p2

  // Compute B^3(u) and B^3(v)
  vec4 B3u = bezierBasis(u);
  vec4 B3v = bezierBasis(v);

  // Compute B^3(u) * G
  vec3 B3uG[4] = vec3[4](vec3(0), vec3(0), vec3(0), vec3(0));
  for (int i=0; i < 4; ++i)
    for (int j=0; j < 4; ++j)
      B3uG[i] += B3u[j] * G[4 * i + j];

  // Compute B^3(u) * G * B^3(v)
  vec3 color = vec3(0);
  for (int i = 0; i < 4; ++i)
    color += B3uG[i] * B3v[i];

  return color;
}

void main() {
  vec2 quadCoords = getQuadCoords();
  gl_Position = vec4(interpolateCoords(quadCoords.x, quadCoords.y), 0, 1);
  colorOut = interpolateColor(quadCoords.x, quadCoords.y);
}

