#version 330 core

layout(location = 0) in vec2 inPos;
layout(location = 1) in vec4 inCol;
out vec4 channelCol;

uniform vec2 offset = vec2(0.0);
uniform float scale = 1.0f;

void main()
{
	gl_Position = vec4((inPos.x + offset.x) * scale, (inPos.y + offset.y) * scale, 0.0, 1.0);
	channelCol = inCol;
}