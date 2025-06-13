#version 330 core

layout(location = 0) in vec3 inPos;
layout(location = 1) in vec2 inUV;

uniform mat4 uM;
uniform mat4 uV;
uniform mat4 uP;

out vec2 fragUV;

void main()
{
    fragUV = inUV;
    gl_Position = uP * uV * uM * vec4(inPos, 1.0);
}