#version 410


uniform mat4 MVP;     // Matrix that contains the result of Projection * View * Model (Matrices)
uniform mat4 NormalMatrix;  // Matrix that used for the normal transformation - it is the inverse transposed of MPV

uniform mat4 V;
uniform mat4 M;

uniform vec3 LightPosition_worldspace;

in vec3 vertexColor;
in vec3 vertexPosition_modelspace;
in vec3 vertexNormal_modelspace;

out vec3 Color;
out vec4 Normal;

//uniform vec3 lightPos;               // Light position in eye coords.

void main()
{


 vec3 Ka = vec3(1,1,1);                // Ambient reflectivity
 vec3 Kd = vec3(1,1,1);                // Diffuse reflectivity
 vec3 Ks = vec3(1,1,1);                // Specular reflectivity

 vec3 AmbientColor = vec3(.2,.2,.2);
 vec3 DiffuseColor = vertexColor;
 vec3 SpecularColor = vec3(1,1,1);

 vec3 LightIntensity = vec3(1,1,1);         // Light source intensity
 float Shininess = 50;        // Specular shininess factor


//    Calculating vertex Position
    gl_Position = MVP * vec4(vertexPosition_modelspace, 1.0);

//    Calculating vertex Normal     (n)
    Normal  = NormalMatrix * vec4(vertexNormal_modelspace ,1.0);
    vec3 n  = normalize(vec3(Normal.xyz/Normal.w));

//    Calculating Light direction   (s)
//    NOTE:In a subtraction of vec4, w maybe affected. So it is needed to first convert to vec3!
    vec4 vEyeCoord4 = V * M * vec4(vertexPosition_modelspace,1.0);
    vec3 vEyeCoord  = vEyeCoord4.xyz/vEyeCoord4.w;
    vec3 s = normalize(LightPosition_worldspace - vEyeCoord);

//    Calculating Ambient Lightning
    vec3 AmbientLightning = LightIntensity * Ka;
    
//    Calculating Diffuse Lightning
    float sDotN = max( dot(s,n), 0.0 );
    vec3 DiffuseLightning = LightIntensity * Kd * sDotN;
    
//    Calculating Specular Lightning
    vec3 SpecularLightning = vec3(0.0);
    vec3 v = -(normalize(vEyeCoord));      //Camera Direction Vector
    vec3 r = normalize(reflect( -s, n ));   //Reflection Vector
    if( sDotN > 0.0 ){
        SpecularLightning = LightIntensity * Ks * pow( max( dot(r,v), 0.0 ), Shininess );
    }

    //Color = AmbientColor*AmbientLightning + DiffuseColor*DiffuseLightning + SpecularColor*SpecularLightning;
    Color = DiffuseColor*DiffuseLightning;
//    vec3 Lightning = vec3(0.0);
//    vec3 Lightning = DiffuseLightning;

    // The diffuse shading equation

//    Color = gridColor * Lightning + vec3(1.0)*SpecularIlluminatio;
//    Color = gridColor * Lightning;
//    Color = SpecularLightning;
//    Color = DiffuseLightning;
//    Color = AmbientLightning;
//    Color = AmbientColor * AmbientLightning;
//    Color = AmbientColor * AmbientLightning + DiffuseColor * DiffuseLightning;

}