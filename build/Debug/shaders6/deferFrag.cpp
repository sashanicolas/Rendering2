#version 330 core

in vec2 UV;

out vec3 color;

uniform sampler2D posicaoTex;
uniform sampler2D corTex;
uniform sampler2D normalTex;

//uniform vec3 lightPos;
uniform float lightPos[150];
uniform float lightCor[150];

//minhas variaveis
vec3 normal;
vec3 position;
vec3 cor;

float numLights;

vec3 getLightPos(int ind){
	return vec3( lightPos[ind*3], lightPos[ind*3+1], lightPos[ind*3+2]);
}
vec3 getLightCor(int ind){
	return vec3( lightCor[ind*3], lightCor[ind*3+1], lightCor[ind*3+2]);
}
vec3 lightInfluence(vec3 lightPos, vec3 lightCor){
	vec3 light = lightPos;
	vec3 lightDir = light - position.xyz ;
	
	normal = normalize(normal);
	lightDir = normalize(lightDir);
	
	vec3 cameraPosition = vec3(0,4.0,8.0);
	vec3 eyeDir = normalize(cameraPosition-position.xyz);
	vec3 vHalfVector = normalize(lightDir.xyz+eyeDir);

	return ((1/numLights)*max(dot(normal,lightDir),0) * cor + pow(max(dot(normal,vHalfVector),0.0), 128))*lightCor;
}

void main(){
	numLights = 50;

	cor = texture( corTex, UV ).xyz;
	position = texture( posicaoTex, UV ).xyz;
	normal = texture( normalTex, UV ).xyz;
	
	vec3 amb = vec3(.2,.2,.2);

	vec3 sumLights = vec3(0,0,0);

	for(int i=0; i<numLights; i++){
		sumLights = sumLights + lightInfluence( getLightPos(i) , getLightCor(i) );	
	}
	
	color = amb*cor + sumLights;
}