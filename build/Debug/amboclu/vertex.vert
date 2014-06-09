#version 120

uniform mat4 modelView;
uniform mat4 projection;
uniform mat4 normalMatrix;
uniform vec4 cor;
uniform vec4 lightPosition;

attribute vec3 position;
attribute vec3 vertexNormal;
attribute float vertexAO;

varying vec4 Cor;
varying vec4 posVertexEye;
varying vec4 normalEye;
varying vec3 dirLuz_eye;
varying vec3 dirCamera_eye;
varying float kAO;

void main(){
    Cor = cor;
    kAO = vertexAO;
    posVertexEye = modelView * vec4(position, 1.0);
	normalEye = normalMatrix * vec4(vertexNormal, 1.0);
	dirLuz_eye = normalize(lightPosition.xyz - posVertexEye.xyz); //direção do vértice a fonte de luz
	dirCamera_eye = normalize(-posVertexEye.xyz); // direção do fragmento para a câmera
    gl_Position = projection * posVertexEye;
}
