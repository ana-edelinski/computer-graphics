#version 330 core 

out vec4 FragColor;
in vec2 chTex;

uniform sampler2D uTex; // Texture sampler

void main()
{
    FragColor = texture(uTex, chTex);
}