#version 330 core

in vec3 color_from_vshader;

out vec3 out_color;

void main() {
	 out_color = color_from_vshader;
}