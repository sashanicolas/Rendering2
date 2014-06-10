#version 330 core

in vec3 vertexPosition_modelspace;
in vec3 vertexColor;
in vec3 vertexNormal_modelspace;

out vec3 color_from_vshader;
out vec3 Position_worldspace;
out vec3 Normal_cameraspace;
out vec3 EyeDirection_cameraspace;
out vec3 LightDirection_cameraspace;


// Values that stay constant for the whole mesh.
uniform mat4 MVP;
uniform mat4 V;
uniform mat4 M;
uniform vec3 LightPosition_worldspace;
uniform mat4 NormalMatrix;

void main() {
	gl_Position =  MVP * vec4(vertexPosition_modelspace,1);
	
	color_from_vshader = vertexColor;

}