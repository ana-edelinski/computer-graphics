#version 330 core
layout(location = 0) in vec3 inPos;

out vec3 TexCoords;

uniform mat4 uV;
uniform mat4 uP;

void main()
{
    vec4 pos = uP * mat4(mat3(uV)) * vec4(inPos, 1.0);
    gl_Position = pos.xyww; // fiksira dubinu na 1
    TexCoords = inPos;
}