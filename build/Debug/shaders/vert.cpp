#version 150

in vec4 position;
in vec4 color;
out vec4 color_from_vshader;

uniform mat4 MVP;

void main() {
	vec4 transformedVector;
	gl_Position = MVP * position;
	color_from_vshader = color;
}