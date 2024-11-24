#version 330 core

in vec2 chTex;          // Teksturne koordinate iz vertex šejdera
out vec4 outCol;        // Izlazna boja

uniform sampler2D uTex; // Uniform za teksturu

void main()
{
    vec4 texColor = texture(uTex, chTex); // Uzmi boju iz teksture
    if (texColor.a < 0.1)                // Proveri alfa kanal
        discard;                         // Discard piksel ako je transparentan
    outCol = texColor;
}
