#version 330 core

in vec2 UV;

out vec3 color;

uniform sampler2D posicaoTex;
uniform sampler2D corTex;
uniform sampler2D normalTex;

//uniform vec3 lightPos;
uniform float lightPos[6];

//minhas variaveis
vec3 normal;
vec3 position;
vec3 cor;

vec3 getLightPos(int ind){
	return vec3(lightPos[ind*3],lightPos[ind*3+1],lightPos[ind*3+2]);
}

vec3 lightInfluence(vec3 lightPos){
	vec3 light = lightPos;
	vec3 lightDir = light - position.xyz ;
	
	normal = normalize(normal);
	lightDir = normalize(lightDir);
	
	vec3 cameraPosition = vec3(0,4.0,8.0);
	vec3 eyeDir = normalize(cameraPosition-position.xyz);
	vec3 vHalfVector = normalize(lightDir.xyz+eyeDir);

	return max(dot(normal,lightDir),0) * cor + pow(max(dot(normal,vHalfVector),0.0), 50) * 1.5;
}

void main(){
	
	cor = texture( corTex, UV ).xyz;
	position = texture( posicaoTex, UV ).xyz;
	normal = texture( normalTex, UV ).xyz;
	
	vec3 amb = vec3(.2,.2,.2);

	vec3 sumLights = vec3(0,0,0);

	sumLights = sumLights + lightInfluence(getLightPos(0));
	sumLights = sumLights + lightInfluence(getLightPos(1));

	color = amb*cor + sumLights;
}