#version 330 core

in vec3 vertexPosition_modelspace;
in vec3 vertexColor;
in vec3 vertexNormal_modelspace;

out vec3 color_from_vshader;
out vec3 normal;
out vec3 LightDirection;

// Values that stay constant for the whole mesh.
uniform mat4 MVP;
uniform mat4 V;
uniform mat4 M;
uniform vec3 LightPosition_worldspace;
uniform mat4 NormalMatrix;

void main() {
/*    vec3 eyeNormal = (NormalMatrix * vec4(vertexNormal_modelspace,1.0)).xyz;
    vec3 p4 = (M * V * vec4(vertexPosition_modelspace,1.0)).xyz;
    
    vec3 LightDirection = normalize(LightPosition_worldspace - p4);
    float diff = max(0.0, dot( eyeNormal, LightDirection));
    
    color_from_vshader = diff * vertexColor;
    gl_Position =  MVP * vec4(vertexPosition_modelspace,1);*/
    
    color_from_vshader = vertexColor;
    normal = (NormalMatrix * vec4(vertexNormal_modelspace,1.0)).xyz;
    vec3 vPosition4 = ( V * M * vec4(vertexPosition_modelspace,1)).xyz;
    //vec3 vPosition3 = vPosition4.xyz / vPosition4.w;
    LightDirection = normalize(LightPosition_worldspace - vPosition4);
    gl_Position =  MVP * vec4(vertexPosition_modelspace,1);
}