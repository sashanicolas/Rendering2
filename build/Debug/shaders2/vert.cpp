#version 330 core

in vec3 vertexPosition_modelspace;
in vec3 vertexColor;
in vec3 vertexNormal_modelspace;

out vec3 color_from_vshader;

// Values that stay constant for the whole mesh.
uniform mat4 MVP;
uniform mat4 V;
uniform mat4 M;
uniform vec3 LightPosition_worldspace;

void main() {
	gl_Position =  MVP * vec4(vertexPosition_modelspace,1);

	// Position of the vertex, in worldspace : M * position
	vec3 Position_worldspace = (M * vec4(vertexPosition_modelspace,1)).xyz;
	
	// Vector that goes from the vertex to the camera, in camera space.
	// In camera space, the camera is at the origin (0,0,0).
	vec3 vertexPosition_cameraspace = ( V * M * vec4(vertexPosition_modelspace,1)).xyz;
//	vec3 EyeDirection_cameraspace = vec3(0,0,0) - vertexPosition_cameraspace;
	vec3 EyeDirection_cameraspace = vertexPosition_cameraspace;

	// Vector that goes from the vertex to the light, in camera space. M is ommited because it's identity.
	vec3 LightPosition_cameraspace = ( V * vec4(LightPosition_worldspace,1)).xyz;
	vec3 LightDirection_cameraspace = LightPosition_cameraspace + EyeDirection_cameraspace;
	
	// Normal of the the vertex, in camera space
	vec3 Normal_cameraspace = ( V * M * vec4(vertexNormal_modelspace,0)).xyz; // Only correct if ModelMatrix does not scale the model ! Use its inverse transpose if not.


	// Light emission properties
	// You probably want to put them as uniforms
	vec3 LightColor = vec3(1,1,1);
	float LightPower = 50.0f;
	
	// Material properties
	vec3 MaterialDiffuseColor = vertexColor;
	vec3 MaterialAmbientColor = vec3(0.2,0.2,0.2) * MaterialDiffuseColor;
	vec3 MaterialSpecularColor = vec3(0.9,0.9,0.9);

	// Distance to the light
	float distance = length( LightPosition_worldspace - Position_worldspace );

	// Normal of the computed fragment, in camera space
	vec3 n = normalize( Normal_cameraspace );
	// Direction of the light (from the fragment to the light)
	vec3 l = normalize( LightDirection_cameraspace );
	// Cosine of the angle between the normal and the light direction, 
	// clamped above 0
	//  - light is at the vertical of the triangle -> 1
	//  - light is perpendicular to the triangle -> 0
	//  - light is behind the triangle -> 0
	float cosTheta = clamp( dot( n,l ), 0,1 );
	
	// Eye vector (towards the camera)
	vec3 E = normalize(EyeDirection_cameraspace);
	// Direction in which the triangle reflects the light
	vec3 R = reflect(-l,n);
	// Cosine of the angle between the Eye vector and the Reflect vector,
	// clamped to 0
	//  - Looking into the reflection -> 1
	//  - Looking elsewhere -> < 1
	float cosAlpha = clamp( dot( E,R ), 0,1 );

	color_from_vshader = 
	// 	// Ambient : simulates indirect lighting
	 	MaterialAmbientColor ;//+
	// 	// Diffuse : "color" of the object
	 	//MaterialDiffuseColor * LightColor * LightPower * cosTheta / (distance*distance) +
	// 	// Specular : reflective highlight, like a mirror
	 	//MaterialSpecularColor * LightColor * LightPower * pow(cosAlpha,9) / (distance*distance);
	
}