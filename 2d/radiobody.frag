#version 330 core

out vec4 FragColor;
uniform vec3 color; // Uniform promenljiva za boju
void main()
{
    FragColor = vec4(color, 1.0); // Koristi uniform za boju
}
