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
uniform mat4 NormalMatrix;

void main() {
	
    vec3 eyeNormal = (NormalMatrix * vec4( vertexNormal_modelspace,1.0)).xyz;
    vec4 pos4 = V * M * vec4(vertexPosition_modelspace,1.0);
    vec3 pos3 = pos4.xyz / pos4.w;
    
    vec3 lightDir = normalize( LightPosition_worldspace - pos3 );
    float diff = max( 0.0, dot( eyeNormal, lightDir ) );

    color_from_vshader = diff * vertexColor;
    color_from_vshader += vec3(.2,.2,.2)*vertexColor;
    
    vec3 reflection = normalize( reflect( -lightDir,eyeNormal ) );
    float spec = max( 0.0, dot( eyeNormal, reflection ) );
    if(diff!=0){
        float fSpec = pow(spec, 128.0);
        color_from_vshader.rgb += vec3(fSpec,fSpec,fSpec);
    }
    
    gl_Position =  MVP * vec4(vertexPosition_modelspace,1.0);
}