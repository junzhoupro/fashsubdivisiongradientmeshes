#version 410
// Vertex shader

layout (location = 0) in vec2 coordsIn;
layout (location = 1) in vec3 colorIn;

uniform vec2 scaling;
uniform vec2 displacement;

layout (location = 0) out vec3 colorOut;

void main() {

    gl_Position = vec4(scaling * (displacement + coordsIn), 0, 1);
    colorOut = colorIn;

}
