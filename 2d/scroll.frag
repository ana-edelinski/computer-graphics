#version 330 core 

out vec4 FragColor;
in vec2 chTex;

uniform sampler2D uTex; 
void main()
{
    FragColor = texture(uTex, chTex);
}
