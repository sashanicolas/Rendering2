#version 330 core

in vec3 color_from_vshader;
in vec3 Position_worldspace;
in vec3 Normal_cameraspace;
in vec3 EyeDirection_cameraspace;
in vec3 LightDirection_cameraspace;

out vec3 out_color;

uniform vec3 LightPosition_worldspace;

void main() {
	
	out_color = color_from_vshader;
			
}