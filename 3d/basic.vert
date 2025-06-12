#version 330 core

layout(location = 0) in vec3 inPos;
layout(location = 1) in vec4 inCol;

uniform mat4 uM; //Matrica transformacije
uniform mat4 uV; //Matrica kamere
uniform mat4 uP; //Matrica projekcija
uniform bool uUseFixedColor; // da li da koristi fiksnu boju - sme li ovo??

out vec4 channelCol;

void main()
{
	gl_Position = uP * uV * uM * vec4(inPos, 1.0); //Zbog nekomutativnosti mnozenja matrica, moramo mnoziti MVP matrice i tjemena "unazad"
	channelCol = inCol;

	if (uUseFixedColor)
		channelCol = vec4(0.3, 0.7, 0.9, 1.0); // fiksna plava
	else
		channelCol = inCol;
}
