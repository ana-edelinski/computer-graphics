#version 330 core

layout(location = 0) in vec2 aPos;

uniform vec2 center;
uniform float scale;

void main() {
    vec2 scaledPos = center + (aPos - center) * scale;
    gl_Position = vec4(scaledPos, 0.0, 1.0);
}
