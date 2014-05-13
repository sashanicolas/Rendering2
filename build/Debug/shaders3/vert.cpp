#version 410

uniform mat4 MVP; 

in vec3 vertexColor;
in vec3 vertexPosition_modelspace;

out vec3 Color;

void main()
{

    gl_Position = MVP * vec4(vertexPosition_modelspace, 1.0);
    Color = vertexColor;
}