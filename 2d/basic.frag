#version 330 core

out vec4 outCol;
in vec4 channelCol;
uniform vec3 color;

void main()
{
	outCol = vec4(color, 1.0);
}