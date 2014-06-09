#version 120

varying vec4 Cor;
varying vec4 posVertexEye; // posição do fragmento no espaço do olho
varying vec4 normalEye; // normal no espaço do olho
varying vec3 dirLuz_eye; //Direção da luz
varying vec3 dirCamera_eye; //Direção da luz
varying float kAO; //Coeficiente de oclusão de ambiente

const float Ns = 128; // expoente especular

void main() {
  vec3 normal_eye = normalize(normalEye.xyz); // vetor normal no espaço do olho
  vec3 halfVector_eye = normalize(dirCamera_eye + dirLuz_eye); // half-vector

  vec4 Ia = kAO * vec4(0.3,0.3,0.3,1.0) * Cor;
  vec4 Id = vec4(0.7,0.7,0.7,1.0) * Cor * max(dot(dirLuz_eye, normal_eye), 0.0);
  vec4 Is = vec4(0.0,0.0,0.0,1.0);

  if(dot(dirLuz_eye, normal_eye)>0)
    Is = vec4(0.9,0.9,0.9,0.9) * pow(max(dot(normal_eye,halfVector_eye), 0), Ns);

  gl_FragColor = Ia;
  //gl_FragColor = Ia + Id + Is;
}
