#version 410
// Tesselation evaluation shader

layout (triangles, equal_spacing, ccw) in;

layout (location = 0) in vec2 coordsIn[];
layout (location = 1) in vec3 colorIn[];
patch in int instance;

uniform vec2 scaling;
uniform vec2 displacement;

layout (location = 0) out vec3 colorOut;

const int N = /* N */;

int wrap(int i) {
  return (i + N) % N;
}

void computeCenter(out vec2 coords, out vec3 color) {
  coords = vec2(0);
  color = vec3(0);
  for (int i = 0; i < N; ++i) {
    coords += coordsIn[5 * i];
    color += colorIn[5 * i];
  }
  coords /= N;
  color /= N;
}

void computeLambda(out float[N] lambda, vec2 v0) {
  // Compute alpha (Paper "Mean value coordinates" figure 1)
  float[N] alpha;
  for (int i = 0; i < N; ++i) {
    vec2 vi = coordsIn[5 * i];
    vec2 vi1 = coordsIn[5 * wrap(i + 1)];
    alpha[i] = acos(dot(normalize(vi - v0), normalize(vi1 - v0)));
  }

  // Compute w (Paper "Mean value coordinates" equation 2.1)
  float sumW = 0;
  float[N] w;
  for (int i = 0; i < N; ++i) {
    vec2 vi = coordsIn[5 * i];
    w[i] = (tan(alpha[wrap(i - 1)] / 2) + tan(alpha[i] / 2)) / length(vi - v0);
    sumW += w[i];
  }

  // Compute lambda (Paper "Mean value coordinates" equation 2.1)
  for (int i = 0; i < N; ++i)
    lambda[i] = w[i] / sumW;
}

void computeSandH(vec2 evalCoords, out float[N] s, out float[N] h) { // TODO: merge this function and next
  float[N] lambda;
  computeLambda(lambda, evalCoords);

  // Compute s and h (Paper "Multisided Generalisations of Gregory Patches" section 2.3)
  for (int i = 0; i < N; ++i) {
    s[i] = lambda[i] / (lambda[i] + lambda[wrap(i - 1)]);
    h[i] = 1 - lambda[i] - lambda[wrap(i - 1)];
  }
}

vec4 bezierBasis(float u) {
  return vec4(pow(1 - u, 3),
              3 * u * pow(1 - u, 2),
              3 * (1 - u) * pow(u, 2),
              pow(u, 3));
}

void computeInterior(vec2 evalCoords, vec2 centerCoords, vec3 centerColor, out vec2 coords, out vec3 color) {
  // Compute s and h (Paper "Multisided Generalisations of Gregory Patches" section 2.3)
  float[N] s, h;
  computeSandH(evalCoords, s, h);

  // Initialize sums
  vec2 sumCoords = vec2(0);
  vec3 sumColor = vec3(0);
  float sumWeights = 0;

  for(int i = 0; i < N; ++i) {
    // Compute mu (Paper "Multisided Generalisations of Gregory Patches" section 2.3)
    float mu1 = h[wrap(i - 1)] / (h[i] + h[wrap(i - 1)]); // case j < 2
    float mu2 = h[wrap(i + 1)] / (h[i] + h[wrap(i + 1)]); // case j > d - 1

    // Compute bezier basis functions
    vec4 bS = bezierBasis(s[i]);
    vec4 bH = bezierBasis(h[i]);

    // Compute product of mu and basis functions (Paper "Multisided Generalisations of Gregory Patches" section 2.3)
    float B00 = mu1 * bS[0] * bH[0];
    float B10 = mu1 * bS[1] * bH[0];
    float B20 = mu2 * bS[2] * bH[0];
    float B30 = mu2 * bS[3] * bH[0];
    float B01 = mu1 * bS[0] * bH[1];
    float B11 = mu1 * bS[1] * bH[1];
    float B21 = mu2 * bS[2] * bH[1];
    float B31 = mu2 * bS[3] * bH[1];

    // Shortcut control points (Paper "Multisided Generalisations of Gregory Patches" figure 4)
    vec2 b00Coords = coordsIn[5 * wrap(i - 1)];
    vec2 b10Coords = coordsIn[5 * wrap(i - 1) + 1];
    vec2 b20Coords = coordsIn[5 * wrap(i - 1) + 2];
    vec2 b30Coords = coordsIn[5 * i];
    vec2 b01Coords = coordsIn[5 * wrap(i - 2) + 2];
    vec2 b11Coords = coordsIn[5 * wrap(i - 1) + 3];
    vec2 b21Coords = coordsIn[5 * wrap(i - 1) + 4];
    vec2 b31Coords = coordsIn[5 * i + 1];

    vec3 b00Color = colorIn[5 * wrap(i - 1)];
    vec3 b10Color = colorIn[5 * wrap(i - 1) + 1];
    vec3 b20Color = colorIn[5 * wrap(i - 1) + 2];
    vec3 b30Color = colorIn[5 * i];
    vec3 b01Color = colorIn[5 * wrap(i - 2) + 2];
    vec3 b11Color = colorIn[5 * wrap(i - 1) + 3];
    vec3 b21Color = colorIn[5 * wrap(i - 1) + 4];
    vec3 b31Color = colorIn[5 * i + 1];

    // Update sums
    sumCoords += B00 * b00Coords + B10 * b10Coords + B20 * b20Coords + B30 * b30Coords + B01 * b01Coords + B11 * b11Coords + B21 * b21Coords + B31 * b31Coords;
    sumColor += B00 * b00Color + B10 * b10Color + B20 * b20Color + B30 * b30Color + B01 * b01Color + B11 * b11Color + B21 * b21Color + B31 * b31Color;
    sumWeights += B00 + B10 + B20 + B30 + B01 + B11 + B21 + B31;
  }

  // Compute point
  coords = sumCoords + (1 - sumWeights) * centerCoords;
  coords = scaling * (displacement + coords);
  color = sumColor + (1 - sumWeights) * centerColor;
}

void main() {
  float u = gl_TessCoord[0];
  float v = gl_TessCoord[1];
  float w = gl_TessCoord[2];

  // Boundary
  if (u == 0) {
    // At corner v(i)
    if(w == 1) {
      gl_Position = vec4(scaling * (displacement + coordsIn[5 * instance]), 0, 1);
      colorOut = colorIn[5 * instance];
      return;
    }

    // At corner v(i - 1)
    if(v == 1) {
      gl_Position = vec4(scaling * (displacement + coordsIn[5 * wrap(instance - 1)]), 0, 1);
      colorOut = colorIn[5 * wrap(instance - 1)];
      return;
    }

    // Along the edge between v(i - 1) to v(i)
    vec4 b = bezierBasis(w);
    vec2 coordsOut = b[0] * coordsIn[5 * wrap(instance - 1)] +
                     b[1] * coordsIn[5 * wrap(instance - 1) + 1] +
                     b[2] * coordsIn[5 * wrap(instance - 1) + 2] +
                     b[3] * coordsIn[5 * instance];
    gl_Position = vec4(scaling * (displacement + coordsOut), 0, 1);
    colorOut = b[0] * colorIn[5 * wrap(instance - 1)] +
               b[1] * colorIn[5 * wrap(instance - 1) + 1] +
               b[2] * colorIn[5 * wrap(instance - 1) + 2] +
               b[3] * colorIn[5 * instance];
    return;
  }

  // Non-boundary
  vec2 centerCoords;
  vec3 centerColor;
  computeCenter(centerCoords, centerColor);

  // Coordinates to compute generalized barycentric coordinate from
  vec2 evalCoords = u * centerCoords + v * coordsIn[5 * wrap(instance - 1)] + w * coordsIn[5 * instance];

  // Compute point
  vec2 coordsOut;
  computeInterior(evalCoords, centerCoords, centerColor, coordsOut, colorOut);
  gl_Position = vec4(coordsOut, 0, 1);
}
