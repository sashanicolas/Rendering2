#version 330 core

in vec3 color_from_vshader;
out vec4 out_color;


void main() {
    out_color = vec4(color_from_vshader,1.0);
}