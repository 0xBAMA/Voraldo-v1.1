#version 430 core

in vec3 normal;
in vec3 color;

out vec4 fragment_output;

void main()
{
	fragment_output = vec4(color, 1);
}
