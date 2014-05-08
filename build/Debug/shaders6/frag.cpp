#version 330 core

in vec4 color_from_vshader;
in vec3 normal;
in vec3 lightDir;

out vec4 out_color;


void main() {
    float diff = max( 0.0, dot( normalize(normal), normalize(lightDir) ) );
    
    out_color = diff * color_from_vshader;
    out_color += vec4(.2,.2,.2,1.0)*color_from_vshader;
    
    vec3 reflection = normalize( reflect( -normalize(lightDir),normalize(normal) ) );
    float spec = max( 0.0, dot( normalize(normal), reflection ) );
    if(diff!=0){
        float fSpec = pow(spec, 32.0);
        out_color.rgb += vec3(fSpec,fSpec,fSpec);
    }
    
}