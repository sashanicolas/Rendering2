#version 330 core

in vec2 UV;

out vec3 color;

uniform sampler2D posicaoTex;
uniform sampler2D corTex;
uniform sampler2D normalTex;

uniform vec3 lightPos;

void main(){
	
	vec3 cor = texture( corTex, UV ).xyz;
	vec3 position = texture( posicaoTex, UV ).xyz;
	vec3 normal = texture( normalTex, UV ).xyz;

	// vec3 light = vec3(0,8.0,6.0);
	vec3 light = lightPos;
	vec3 lightDir = light - position.xyz ;
	
	normal = normalize(normal);
	lightDir = normalize(lightDir);
	
	vec3 cameraPosition = vec3(0,4.0,8.0);
	vec3 eyeDir = normalize(cameraPosition-position.xyz);
	vec3 vHalfVector = normalize(lightDir.xyz+eyeDir);
	
	vec3 amb = vec3(.2,.2,.2);
	color = max(dot(normal,lightDir),0) * cor + pow(max(dot(normal,vHalfVector),0.0), 50) * 1.5 + amb*cor;
	// color = normal;
}