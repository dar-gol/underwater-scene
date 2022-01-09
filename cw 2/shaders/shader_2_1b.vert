#version 430 core

layout(location = 0) in vec4 vertexPosition;
layout(location = 1) in float vertexHue;

out float hue;
void main()
{
	hue = vertexHue;
	gl_Position = vertexPosition;
}
