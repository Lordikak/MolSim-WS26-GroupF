#version 430

in vec2 vUV;
out vec4 fragColor;

void main() {
    fragColor = vec4(vUV * 0.5 + 0.5, 0.0, 1.0);
    if (dot(vUV, vUV) > 1.0) discard;
}