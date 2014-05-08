#version 330 core

in vec3 vertexPosition_modelspace;
in vec3 vertexColor;
in vec3 vertexNormal_modelspace;

out vec4 color_from_vshader;
out vec3 normal;
out vec3 lightDir;

// Values that stay constant for the whole mesh.
uniform mat4 MVP;
uniform mat4 V;
uniform mat4 M;
uniform vec3 LightPosition_worldspace;
uniform mat4 NormalMatrix;

void main() {
	
    normal = (NormalMatrix * vec4( vertexNormal_modelspace,1.0)).xyz;
    vec4 pos4 = V * M * vec4(vertexPosition_modelspace,1.0);
    vec3 pos3 = pos4.xyz / pos4.w;
    
    lightDir = normalize( LightPosition_worldspace - pos3 );
    
    gl_Position =  MVP * vec4(vertexPosition_modelspace,1.0);
    
    color_from_vshader = vec4(vertexColor,1.0);
}