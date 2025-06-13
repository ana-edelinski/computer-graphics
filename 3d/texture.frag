#version 330 core

in vec2 fragUV;
uniform sampler2D uTex;

out vec4 outColor;

void main()
{
    outColor = texture(uTex, fragUV);
}