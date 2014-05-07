#version 330 core

in vec3 color_from_vshader;
in vec3 normal;
in vec3 LightDirection;

out vec3 out_color;

void main() {
    out_color = color_from_vshader;
    float diff = max(0.0, dot( normalize(normal), normalize(LightDirection) ));
	//out_color = diff * color_from_vshader;
    //out_color += vec3(0.2,0.2,0.2) * color_from_vshader;
    vec3 vReflection = normalize(reflect(-normalize(LightDirection),normalize(normal)));
    float spec = max(0.0, dot(normalize(normal),vReflection));
    if(diff != 0){
        float fSpec = pow(spec,128.0);
        out_color.rgb += vec3(fSpec,fSpec,fSpec);
    }
}