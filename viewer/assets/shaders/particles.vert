#version 430
layout(std430, binding = 0) readonly buffer Particles { vec4 p[]; };
uniform mat4 uView, uProj;
uniform float uRadius;
out vec2 vUV;

const vec2 kVerts[6] = vec2[](
    vec2(-1,-1), vec2( 1,-1), vec2( 1, 1),
    vec2(-1,-1), vec2( 1, 1), vec2(-1, 1)
);

void main() {
    vec2 v = kVerts[gl_VertexID % 6];
    vec4 center = uView * vec4(p[gl_VertexID / 6].xyz, 1.0);
    vUV = v;
    gl_Position = uProj *(center + vec4(v * uRadius, 0, 0));
}