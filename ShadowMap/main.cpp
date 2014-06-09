// Draw four triangles on a red background
#include <GL/glew.h>
#include <GL/glfw.h>
#include <cstdlib>
#include <iostream>
#include <fstream>
#include <vector>
#include <ctime>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <map>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
//#include <glm/gtx/transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/rotate_vector.hpp>

#include "myutils.h"

#define ARRAY_SIZE_IN_ELEMENTS(a) (sizeof(a)/sizeof(a[0]))
#define ZERO_MEM(a) memset(a, 0, sizeof(a))
#define WINDOW_WIDTH  800
#define WINDOW_HEIGHT 800

//Classes
struct Camera{
    float x, y, z;
};
Camera c;

class Luz{
public:
    glm::vec3 pos;
    glm::vec3 cor;
    
    Luz(glm::vec3 pos, glm::vec3 cor){
        this->pos = pos;
        this->cor = cor;
    }
    
    void setPos(glm::vec3 pos){
        this->pos = pos;
    }
};

class Shader{
public:
    //================ Variaveis ================//
    // id para o programa e os shaders
    GLuint shaderProgram, vertexShader, fragmentShader;
    glm::mat4 Model;
    
    //matrizes
    glm::mat4 Projection, View, MVP, ITMV;
    //IDs
    GLint mvp, itmv, v, LightID, m;
    //luz
    glm::vec3 lightPos;
    
    //================ Metodos ================//
    Shader (){
        
    }
    Shader (const char* v, const char* f){
        // create a program
        this->VertexShader(v);
        this->FragmentShader(f);
        
        shaderProgram = glCreateProgram();
        glAttachShader(shaderProgram, vertexShader);
        glAttachShader(shaderProgram, fragmentShader);
        
        // Flag the shaders for deletion
        glDeleteShader(vertexShader);
        glDeleteShader(fragmentShader);
        
        this->Link();
        
    }
    ~Shader (){
        
    }
    
    GLuint programID(){
        return shaderProgram;
    }
    
    bool VertexShader (const char* source){
        vertexShader = load_and_compile_shader(source, GL_VERTEX_SHADER);
        
        return true;
    }
    bool FragmentShader (const char* source){
        fragmentShader = load_and_compile_shader(source, GL_FRAGMENT_SHADER);
        
        return true;
    }
    bool Link (){
        // Link and use the program
        glLinkProgram(shaderProgram);
        glUseProgram(shaderProgram);
        
        return true;
    }
    
    bool Validate ();
    const char* GetMessage ();
    void Load (){
        glUseProgram(shaderProgram);
    }
    void Unload ();
    void SetUniform (const char* name, float x);
    void SetUniform (const char* name, int id){
        GLuint ShadowMapID = glGetUniformLocation(shaderProgram, "shadowMap");
        glUniform1i(ShadowMapID, id);
    }
    void SetUniform (const char* name, float x, float y);
    void SetUniform (const char* name, float x, float y, float z){
        GLint ID = glGetUniformLocation(shaderProgram, name );
        glUniform3f(ID, x, y, z);
    }
    void SetUniform (const char* name, float x, float y, float z, float w);
    void SetUniform (const char* name, int size, int count, float* v);
    void SetUniformI (const char* name, int x);
    void SetUniformI (const char* name, int x, int y);
    void SetUniformI (const char* name, int x, int y, int z);
    void SetUniformI (const char* name, int x, int y, int z, int w);
    void SetUniformI (const char* name, int size, int count, int* v) ;
    void SetUniformMatrix (const char* name, int row, int col, int count, float* v);
    
    void SetUniform (const char* name, glm::mat4 mat){
        // Transfer the transformation matrices to the shader program
        m = glGetUniformLocation(shaderProgram, name );
        glUniformMatrix4fv(m, 1, GL_FALSE, glm::value_ptr(mat));
        
    }
    
    void setModelMatrix(glm::mat4 Model){
        this->Model = Model;
    }
    void setProjectionMatrix(glm::mat4 P){
        this->Projection = P;
    }
    void setViewMatrix(glm::mat4 V){
        this->View = V;
    }
    
    void SetUniformMVP(){
        // Projection matrix : 45° Field of View, 4:3 ratio, display range : 0.1 unit <-> 100 units
        Projection = glm::perspective(45.0f, 1.0f/*4.0f / 3.0f*/, 0.1f, 100.0f);
        
        // Camera matrix
        View = glm::lookAt(
                           //                           glm::vec3(c.x,c.y,c.z), // Camera is at (x,y,z), in World Space, bom = 0,-2.0f,4.0f
                           glm::vec3(0,4,8), // Camera is at (x,y,z), in World Space, bom = 0,-2.0f,4.0f
                           glm::vec3(0, 0, 0), // and looks at the origin
                           glm::vec3(0,1.0f,0) // Head is up (set to 0,-1,0 to look upside-down)
                           );
        
        // Our ModelViewProjection : multiplication of our 3 matrices
        MVP = Projection * View * Model; // Remember, matrix multiplication is the other way around
        //glm::mat4 MVP = glm::mat4(1.0f);
        
        ITMV = glm::transpose(glm::inverse(View * Model));
        
        // Transfer the transformation matrices to the shader program
        mvp = glGetUniformLocation(shaderProgram, "MVP" );
        glUniformMatrix4fv(mvp, 1, GL_FALSE, glm::value_ptr(MVP));
        
        // Transfer the transformation matrices to the shader program
        itmv = glGetUniformLocation(shaderProgram, "NormalMatrix" );
        glUniformMatrix4fv(itmv, 1, GL_FALSE, glm::value_ptr(ITMV));
        
        // Transfer the transformation matrices to the shader program
        v = glGetUniformLocation(shaderProgram, "V" );
        glUniformMatrix4fv(v, 1, GL_FALSE, glm::value_ptr(View));
        
        //        lightPos = glm::vec3(0,7,7);//bom = 5.0f, 4.0f, 4.0f
        lightPos = glm::vec3(c.x,c.y,c.z);//bom = 5.0f, 4.0f, 4.0f
        LightID = glGetUniformLocation(shaderProgram, "LightPosition_worldspace" );
		glUniform3f(LightID, lightPos.x, lightPos.y, lightPos.z);
    }
    
    void SetUniformMVP2(){
        
        ITMV = glm::transpose(glm::inverse(View * Model));
        
//        // Transfer the transformation matrices to the shader program
//        mvp = glGetUniformLocation(shaderProgram, "MVP" );
//        glUniformMatrix4fv(mvp, 1, GL_FALSE, glm::value_ptr(MVP));
//        
        // Transfer the transformation matrices to the shader program
        itmv = glGetUniformLocation(shaderProgram, "NormalMatrix" );
        glUniformMatrix4fv(itmv, 1, GL_FALSE, glm::value_ptr(ITMV));
        
        
    }
    
    // Read a shader source from a file
    // store the shader source in a std::vector<char>
    void read_shader_src(const char *fname, std::vector<char> &buffer) {
        std::ifstream in;
        in.open(fname, std::ios::binary);
        
        if(in.is_open()) {
            // Get the number of bytes stored in this file
            in.seekg(0, std::ios::end);
            size_t length = (size_t)in.tellg();
            
            // Go to start of the file
            in.seekg(0, std::ios::beg);
            
            // Read the content of the file in a buffer
            buffer.resize(length + 1);
            in.read(&buffer[0], length);
            in.close();
            // Add a valid C - string end
            buffer[length] = '\0';
        }
        else {
            std::cerr << "Unable to open " << fname << " I'm out!" << std::endl;
            exit(-1);
        }
    }
    
    // Compile a shader
    GLuint load_and_compile_shader(const char *fname, GLenum shaderType) {
        // Load a shader from an external file
        std::vector<char> buffer;
        read_shader_src(fname, buffer);
        const char *src = &buffer[0];
        
        // Compile the shader
        GLuint shader = glCreateShader(shaderType);
        glShaderSource(shader, 1, &src, NULL);
        glCompileShader(shader);
        // Check the result of the compilation
        GLint test;
        glGetShaderiv(shader, GL_COMPILE_STATUS, &test);
        if(!test) {
            std::cerr << "Shader compilation failed with this message:" << std::endl;
            std::vector<char> compilation_log(512);
            glGetShaderInfoLog(shader, compilation_log.size(), NULL, &compilation_log[0]);
            std::cerr << &compilation_log[0] << std::endl;
            glfwTerminate();
            exit(-1);
        }
        return shader;
    }
    
    
};

class Mesh{
public:
    std::vector<glm::vec3> vertices;
	std::vector<glm::vec2> uvs;
    std::vector<glm::vec2> colors;
	std::vector<glm::vec3> normals;
    //buffers
    GLuint vertexbuffer, colorsbuffer, normalsbuffer;
    
    Mesh (){
        // Create a Vector Buffer Object that will store the vertices on video memory
        glGenBuffers(1, &vertexbuffer);
        glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
        glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(glm::vec3), &vertices[0], GL_STATIC_DRAW);
        
        // Create a Vector Buffer Object that will store the colors on video memory
        glGenBuffers(1, &colorsbuffer);
        glBindBuffer(GL_ARRAY_BUFFER, colorsbuffer);
        glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(glm::vec3), &colors[0], GL_STATIC_DRAW);
        
        // Create a Vector Buffer Object that will store the colors on video memory
        glGenBuffers(1, &normalsbuffer);
        glBindBuffer(GL_ARRAY_BUFFER, normalsbuffer);
        glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(glm::vec3), &normals[0], GL_STATIC_DRAW);
        
    }
    ~Mesh();
    
    int numberOfPoints(){
        return vertices.size();
    }

};

class Grid {
    // Create a grid with nx x ny quadrilateral.
    // The grid coordinates varies from 0.0 to 1.0 in the xz plane
public:
    int m_nx, m_ny;
    GLfloat vertices_position[500000];
    GLfloat colors[500000];
    GLfloat normals[500000];
    glm::mat4 Model;
    glm::mat4 depthMVP;
    
    //buffers
    GLuint vertexbuffer, colorsbuffer, normalsbuffer;
    
    Grid (int nx, int ny){
        this->m_nx = nx;
        this->m_ny = ny;
        
        this->genGrid();
        
        // Create a Vector Buffer Object that will store the vertices on video memory
        glGenBuffers(1, &vertexbuffer);
        glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
        glBufferData(GL_ARRAY_BUFFER, this->sizeOfAllFloats(), this->vertices_position, GL_STATIC_DRAW);
        
        // Create a Vector Buffer Object that will store the colors on video memory
        glGenBuffers(1, &colorsbuffer);
        glBindBuffer(GL_ARRAY_BUFFER, colorsbuffer);
        glBufferData(GL_ARRAY_BUFFER, this->sizeOfAllFloats(), this->colors, GL_STATIC_DRAW);
        
        // Create a Vector Buffer Object that will store the colors on video memory
        glGenBuffers(1, &normalsbuffer);
        glBindBuffer(GL_ARRAY_BUFFER, normalsbuffer);
        glBufferData(GL_ARRAY_BUFFER, this->sizeOfAllFloats(), this->normals, GL_STATIC_DRAW);
        
    }
    
    // Destroy a grid
    ~Grid ();
    
    void genGrid(){
        for (int j=0; j<m_ny; j++) {
            for (int i=0; i<m_nx; i++) {
                int k = Index(i, j);
                vertices_position[18*k] = (float)i / (float)m_nx;
                vertices_position[18*k+1] = 0.0f;
                vertices_position[18*k+2] = (float)j / (float)m_ny;
                
                vertices_position[18*k+3] = ((float)i+1) / (float)m_nx;
                vertices_position[18*k+4] = 0.0f;
                vertices_position[18*k+5] = ((float)j+1) / (float)m_ny;
                
                vertices_position[18*k+6] = ((float)i+1) / (float)m_nx;
                vertices_position[18*k+7] = 0.0f;
                vertices_position[18*k+8] = (float)j / (float)m_ny;
                
                
                vertices_position[18*k+9] = ((float)i+1) / (float)m_nx;
                vertices_position[18*k+10] = 0.0f;
                vertices_position[18*k+11] = ((float)j+1) / (float)m_ny;
                
                vertices_position[18*k+12] = (float)i / (float)m_nx;
                vertices_position[18*k+13] = 0.0f;
                vertices_position[18*k+14] = (float)j / (float)m_ny;
                
                vertices_position[18*k+15] = (float)i / (float)m_nx;
                vertices_position[18*k+16] = 0.0f;
                vertices_position[18*k+17] = ((float)j+1) / (float)m_ny;
            }
        }
        int k = 0;
        for (int j=0; j<m_nx*m_ny*6; j++) {
            colors[k++]=0.4f;
            colors[k++]=0.6f;
            colors[k++]=0;
        }
        k = 0;
        for (int j=0; j<m_nx*m_ny*6; j++) {
            normals[k++]=0;
            normals[k++]=1;
            normals[k++]=0;
        }
        /*
         for (int j=0; j<m_ny; j++) {
         for (int i=0; i<m_nx; i++) {
         int k = Index(i, j);
         printf("i=%d j=%d\n",i,j);
         
         printf("triangulo 1\n");
         printf("(%.2f,%.2f,%.2f)\n",vertices_position[18*k],vertices_position[18*k+1],vertices_position[18*k+2]);
         printf("(%.2f,%.2f,%.2f)\n",vertices_position[18*k+3],vertices_position[18*k+4],vertices_position[18*k+5]);
         printf("(%.2f,%.2f,%.2f)\n",vertices_position[18*k+6],vertices_position[18*k+7],vertices_position[18*k+8]);
         
         printf("triangulo 2\n");
         printf("(%.2f,%.2f,%.2f)\n",vertices_position[18*k+9],vertices_position[18*k+10],vertices_position[18*k+11]);
         printf("(%.2f,%.2f,%.2f)\n",vertices_position[18*k+12],vertices_position[18*k+13],vertices_position[18*k+14]);
         printf("(%.2f,%.2f,%.2f)\n\n",vertices_position[18*k+15],vertices_position[18*k+16],vertices_position[18*k+17]);
         }
         }*/
        
    }
    
    void draw(Shader *myShader){
        GLuint shaderProgram = myShader->programID();
        
        // Use our shader
		glUseProgram(myShader->programID());
        
        // 1rst attribute buffer : vertices
        GLint position_attribute = glGetAttribLocation(shaderProgram, "vertexPosition_modelspace");
        glEnableVertexAttribArray(position_attribute);
        glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
        glVertexAttribPointer(
                              position_attribute,                  // attribute
                              3,                  // size
                              GL_FLOAT,           // type
                              GL_FALSE,           // normalized?
                              0,                  // stride
                              (void*)0            // array buffer offset
                              );
        
        // 2nd attribute buffer : colorsbuffer
        GLint color_attribute = glGetAttribLocation(shaderProgram, "vertexColor");
        glEnableVertexAttribArray(color_attribute);
        glBindBuffer(GL_ARRAY_BUFFER, colorsbuffer);
        glVertexAttribPointer(
                              color_attribute,                                // attribute
                              3,                                // size
                              GL_FLOAT,                         // type
                              GL_FALSE,                         // normalized?
                              0,                                // stride
                              (void*)0                          // array buffer offset
                              );
        
        // 3rd attribute buffer : normals
        GLint normal_attribute = glGetAttribLocation(shaderProgram, "vertexNormal_modelspace");
        glEnableVertexAttribArray(normal_attribute);
        glBindBuffer(GL_ARRAY_BUFFER, normalsbuffer);
        glVertexAttribPointer(
                              normal_attribute,                                // attribute
                              3,                                // size
                              GL_FLOAT,                         // type
                              GL_FALSE,                         // normalized?
                              0,                                // stride
                              (void*)0                          // array buffer offset
                              );
        // Model matrix : an identity matrix (model will be at the origin)
        Model = glm::mat4(1.0f); // Changes for each model !
        Model = glm::translate(Model, glm::vec3(-4.0f, 0, -4.0f));
        Model = glm::scale(Model, glm::vec3(8,1,8));
        
        myShader->setModelMatrix(Model);
        myShader->SetUniform("M", Model);
        myShader->SetUniformMVP();
        glDrawArrays(GL_TRIANGLES, 0, this->numberOfPoints());
        
        glDisableVertexAttribArray(position_attribute);
        glDisableVertexAttribArray(color_attribute);
        glDisableVertexAttribArray(normal_attribute);
        
    }
    
    void drawDepthShader(Shader *myShader){
        GLuint shaderProgram = myShader->programID();
        
        // Use our shader
		glUseProgram(myShader->programID());
        
        // 1rst attribute buffer : vertices
        GLint position_attribute = glGetAttribLocation(shaderProgram, "vertexPosition_modelspace");
        glEnableVertexAttribArray(position_attribute);
        glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
        glVertexAttribPointer(
                              position_attribute,                  // attribute
                              3,                  // size
                              GL_FLOAT,           // type
                              GL_FALSE,           // normalized?
                              0,                  // stride
                              (void*)0            // array buffer offset
                              );
        
        // Model matrix : an identity matrix (model will be at the origin)
        Model = glm::mat4(1.0f); // Changes for each model !
        Model = glm::translate(Model, glm::vec3(-4.0f, 0, -4.0f));
        Model = glm::scale(Model, glm::vec3(8,1,8));
        
        myShader->setModelMatrix(Model);
        
        
        glm::mat4 depthModelMatrix = Model;
        depthMVP = myShader->Projection * myShader->View * depthModelMatrix;
        // Send our transformation to the currently bound shader,
        // in the "MVP" uniform
        myShader->SetUniform("depthMVP", depthMVP);
        
        
        //myShader->SetUniformMVP2();
        glDrawArrays(GL_TRIANGLES, 0, this->numberOfPoints());
        
        glDisableVertexAttribArray(position_attribute);
    }
    
    void drawShadowShader(Shader *myShader, GLuint textura){
        GLuint shaderProgram = myShader->programID();
        
        // Use our shader
		glUseProgram(myShader->programID());
        
        glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, textura);
        GLuint ShadowMapID = glGetUniformLocation(myShader->programID(), "shadowMap");
        glUniform1i(ShadowMapID, 0);
        
        // 1rst attribute buffer : vertices
        GLint position_attribute = glGetAttribLocation(shaderProgram, "vertexPosition_modelspace");
        glEnableVertexAttribArray(position_attribute);
        glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
        glVertexAttribPointer(
                              position_attribute,                  // attribute
                              3,                  // size
                              GL_FLOAT,           // type
                              GL_FALSE,           // normalized?
                              0,                  // stride
                              (void*)0            // array buffer offset
                              );
        
        // 2nd attribute buffer : colorsbuffer
        GLint color_attribute = glGetAttribLocation(shaderProgram, "vertexColor");
        glEnableVertexAttribArray(color_attribute);
        glBindBuffer(GL_ARRAY_BUFFER, colorsbuffer);
        glVertexAttribPointer(
                              color_attribute,                                // attribute
                              3,                                // size
                              GL_FLOAT,                         // type
                              GL_FALSE,                         // normalized?
                              0,                                // stride
                              (void*)0                          // array buffer offset
                              );
        
        // 3rd attribute buffer : normals
        GLint normal_attribute = glGetAttribLocation(shaderProgram, "vertexNormal_modelspace");
        glEnableVertexAttribArray(normal_attribute);
        glBindBuffer(GL_ARRAY_BUFFER, normalsbuffer);
        glVertexAttribPointer(
                              normal_attribute,                                // attribute
                              3,                                // size
                              GL_FLOAT,                         // type
                              GL_FALSE,                         // normalized?
                              0,                                // stride
                              (void*)0                          // array buffer offset
                              );
        // Model matrix : an identity matrix (model will be at the origin)
        Model = glm::mat4(1.0f); // Changes for each model !
        Model = glm::translate(Model, glm::vec3(-4.0f, 0, -4.0f));
        Model = glm::scale(Model, glm::vec3(8,1,8));
        
        myShader->setModelMatrix(Model);
        
        glm::mat4 MVP = myShader->Projection * myShader->View * Model;
        glm::mat4 biasMatrix(
                             0.5, 0.0, 0.0, 0.0,
                             0.0, 0.5, 0.0, 0.0,
                             0.0, 0.0, 0.5, 0.0,
                             0.5, 0.5, 0.5, 1.0
                             );
        glm::mat4 depthBiasMVP = biasMatrix * depthMVP;
        
        // Send our transformation to the currently bound shader,
        // in the "MVP" uniform
        myShader->SetUniform("M", Model);
        myShader->SetUniform("MVP", MVP);
        myShader->SetUniform("DepthBiasMVP", depthBiasMVP);
        
        myShader->SetUniformMVP2();
        
        glDrawArrays(GL_TRIANGLES, 0, this->numberOfPoints());
        
        glDisableVertexAttribArray(position_attribute);
        glDisableVertexAttribArray(color_attribute);
        glDisableVertexAttribArray(normal_attribute);
        
    }
    
    int sizeOfAllFloats(){
        return m_nx*m_ny*6*3*sizeof(GLfloat); //10 vezes 10 sub quadrados com dois triangulos. Cada triangulo 3 vertices. Cada Vertice sao 3 floats
    }
    
    int Index (int i, int j) {
        return i + j*(m_ny);
    }
    
    int numberOfPoints(){
        return m_nx*m_ny*2*3;
    }
};

class Sphere {
    // Create a sphere with the specified number of subdivisions
    // The sphere is centered at the origin with radius 1.0
public:
    int m_nx, m_ny;
    const float PI = 3.1415926;
    
    GLfloat vertices_position[500000];
    GLfloat colors[500000];
    GLfloat normals[500000];
    glm::mat4 Model;
    glm::mat4 depthMVP;
    float x, y, z;
    
    //buffers
    GLuint vertexbuffer, colorsbuffer, normalsbuffer;
    
    Sphere (int slices, int stack){
        this->m_nx = slices;
        this->m_ny = stack;
        y=0.2f;
        
        //sector = slice
        //ring = stack
        
        this->genSphere();
        
        // Create a Vector Buffer Object that will store the vertices on video memory
        glGenBuffers(1, &vertexbuffer);
        glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
        glBufferData(GL_ARRAY_BUFFER, this->sizeOfAllFloats(), this->vertices_position, GL_STATIC_DRAW);
        
        // Create a Vector Buffer Object that will store the colors on video memory
        glGenBuffers(1, &colorsbuffer);
        glBindBuffer(GL_ARRAY_BUFFER, colorsbuffer);
        glBufferData(GL_ARRAY_BUFFER, this->sizeOfAllFloats(), this->colors, GL_STATIC_DRAW);
        
        // Create a Vector Buffer Object that will store the colors on video memory
        glGenBuffers(1, &normalsbuffer);
        glBindBuffer(GL_ARRAY_BUFFER, normalsbuffer);
        glBufferData(GL_ARRAY_BUFFER, this->sizeOfAllFloats(), this->normals, GL_STATIC_DRAW);
    }
    
    // Destroy a sphere
    ~Sphere ();
    
    
    void setPositionXZ(int i,int j){
        x = 0.8*i - 3.6f;
        z = 0.8*j - 3.6f;
    }
    
    // Draw a sphere
    void genSphere (){
        
        
        for (int j=0; j<m_ny; j++) {
            for (int i=0; i<m_nx; i++) {
                int k = Index(i, j);
                vertices_position[18*k] = cos( 2*PI*((float)i / (float)m_nx)) * sin( PI*((float)j / (float)m_nx) ) ;
                vertices_position[18*k+1] = cos( PI*((float)j / (float)m_nx) );
                vertices_position[18*k+2] = sin( 2*PI*((float)i / (float)m_nx) ) * sin( PI*((float)j / (float)m_nx) ) ;
                
                vertices_position[18*k+3] = cos( 2*PI*(((float)i+1) / (float)m_nx)) * sin( PI*((float)j / (float)m_nx) ) ;
                vertices_position[18*k+4] = cos( PI*((float)j / (float)m_nx) );
                vertices_position[18*k+5] = sin( 2*PI*(((float)i+1) / (float)m_nx) ) * sin( PI*((float)j / (float)m_nx) ) ;
                
                vertices_position[18*k+6] = cos( 2*PI*(((float)i+1) / (float)m_nx)) * sin( PI*(((float)j+1) / (float)m_nx) ) ;
                vertices_position[18*k+7] = cos( PI*(((float)j+1) / (float)m_nx) );
                vertices_position[18*k+8] = sin( 2*PI*(((float)i+1) / (float)m_nx) ) * sin( PI*(((float)j+1) / (float)m_nx) ) ;
                
                
                vertices_position[18*k+9] = cos( 2*PI*((float)i / (float)m_nx)) * sin( PI*((float)j / (float)m_nx) ) ;
                vertices_position[18*k+10] = cos( PI*((float)j / (float)m_nx) );
                vertices_position[18*k+11] = sin( 2*PI*((float)i / (float)m_nx) ) * sin( PI*((float)j / (float)m_nx) ) ;
                
                vertices_position[18*k+12] = cos( 2*PI*(((float)i+1) / (float)m_nx)) * sin( PI*(((float)j+1) / (float)m_nx) ) ;
                vertices_position[18*k+13] = cos( PI*(((float)j+1) / (float)m_nx) );
                vertices_position[18*k+14] = sin( 2*PI*(((float)i+1) / (float)m_nx) ) * sin( PI*(((float)j+1) / (float)m_nx) ) ;
                
                vertices_position[18*k+15] = cos( 2*PI*((float)i / (float)m_nx)) * sin( PI*(((float)j+1) / (float)m_nx) ) ;
                vertices_position[18*k+16] = cos( PI*(((float)j+1) / (float)m_nx) );
                vertices_position[18*k+17] = sin( 2*PI*((float)i / (float)m_nx) ) * sin( PI*(((float)j+1) / (float)m_nx) ) ;
            }
        }
        
        int k = 0;
        for (int j=0; j<m_nx*m_ny*6; j++) {
            colors[k++]=1;
            colors[k++]=0;
            colors[k++]=0;
        }
        
        
        for (int j=0; j<m_ny*m_nx*6*3; j++) {
            normals[j] = vertices_position[j];
        }
        
        
    }//end gen
    
    void draw(Shader *myShader){
        GLuint shaderProgram = myShader->programID();
        
        // Use our shader
		glUseProgram(myShader->programID());
        
        // 1rst attribute buffer : vertices
        GLint position_attribute = glGetAttribLocation(shaderProgram, "vertexPosition_modelspace");
        glEnableVertexAttribArray(position_attribute);
        glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
        glVertexAttribPointer(
                              position_attribute,                  // attribute
                              3,                  // size
                              GL_FLOAT,           // type
                              GL_FALSE,           // normalized?
                              0,                  // stride
                              (void*)0            // array buffer offset
                              );
        
        // 2nd attribute buffer : colorsbuffer
        GLint color_attribute = glGetAttribLocation(shaderProgram, "vertexColor");
        glEnableVertexAttribArray(color_attribute);
        glBindBuffer(GL_ARRAY_BUFFER, colorsbuffer);
        glVertexAttribPointer(
                              color_attribute,                                // attribute
                              3,                                // size
                              GL_FLOAT,                         // type
                              GL_FALSE,                         // normalized?
                              0,                                // stride
                              (void*)0                          // array buffer offset
                              );
        
        // 3rd attribute buffer : normals
        GLint normal_attribute = glGetAttribLocation(shaderProgram, "vertexNormal_modelspace");
        glEnableVertexAttribArray(normal_attribute);
        glBindBuffer(GL_ARRAY_BUFFER, normalsbuffer);
        glVertexAttribPointer(
                              normal_attribute,                                // attribute
                              3,                                // size
                              GL_FLOAT,                         // type
                              GL_FALSE,                         // normalized?
                              0,                                // stride
                              (void*)0                          // array buffer offset
                              );
        // Model matrix : an identity matrix (model will be at the origin)
        Model = glm::mat4(1.0f); // Changes for each model !
        Model = glm::translate(Model, glm::vec3(x, y, z));
        Model = glm::scale(Model, glm::vec3(0.2f,0.2f,0.2f));
        
        myShader->setModelMatrix(Model);
        myShader->SetUniform("M", Model);
        myShader->SetUniformMVP();
        glDrawArrays(GL_TRIANGLES, 0, this->numberOfPoints());
        
        glDisableVertexAttribArray(position_attribute);
        glDisableVertexAttribArray(color_attribute);
        glDisableVertexAttribArray(normal_attribute);
        
    }
    
    void drawDepthShader(Shader *myShader){
        GLuint shaderProgram = myShader->programID();
        
        // Use our shader
		glUseProgram(myShader->programID());
        
        // 1rst attribute buffer : vertices
        GLint position_attribute = glGetAttribLocation(shaderProgram, "vertexPosition_modelspace");
        glEnableVertexAttribArray(position_attribute);
        glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
        glVertexAttribPointer(
                              position_attribute,                  // attribute
                              3,                  // size
                              GL_FLOAT,           // type
                              GL_FALSE,           // normalized?
                              0,                  // stride
                              (void*)0            // array buffer offset
                              );
        
        // Model matrix : an identity matrix (model will be at the origin)
        Model = glm::mat4(1.0f); // Changes for each model !
        Model = glm::translate(Model, glm::vec3(x, y, z));
        Model = glm::scale(Model, glm::vec3(0.2f,0.2f,0.2f));
        
        myShader->setModelMatrix(Model);

        
        glm::mat4 depthModelMatrix = Model;
        glm::mat4 depthMVP = myShader->Projection * myShader->View * depthModelMatrix;
        // Send our transformation to the currently bound shader,
        // in the "MVP" uniform
        myShader->SetUniform("depthMVP", depthMVP);
        
        
        //myShader->SetUniformMVP2();
        glDrawArrays(GL_TRIANGLES, 0, this->numberOfPoints());
        
        glDisableVertexAttribArray(position_attribute);
    }
    
    void drawShadowShader(Shader *myShader, GLuint textura){
        GLuint shaderProgram = myShader->programID();
        
        // Use our shader
		glUseProgram(myShader->programID());
        
        glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, textura);
        GLuint ShadowMapID = glGetUniformLocation(myShader->programID(), "shadowMap");
        glUniform1i(ShadowMapID, 0);
        
        // 1rst attribute buffer : vertices
        GLint position_attribute = glGetAttribLocation(shaderProgram, "vertexPosition_modelspace");
        glEnableVertexAttribArray(position_attribute);
        glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
        glVertexAttribPointer(
                              position_attribute,                  // attribute
                              3,                  // size
                              GL_FLOAT,           // type
                              GL_FALSE,           // normalized?
                              0,                  // stride
                              (void*)0            // array buffer offset
                              );
        
        // 2nd attribute buffer : colorsbuffer
        GLint color_attribute = glGetAttribLocation(shaderProgram, "vertexColor");
        glEnableVertexAttribArray(color_attribute);
        glBindBuffer(GL_ARRAY_BUFFER, colorsbuffer);
        glVertexAttribPointer(
                              color_attribute,                                // attribute
                              3,                                // size
                              GL_FLOAT,                         // type
                              GL_FALSE,                         // normalized?
                              0,                                // stride
                              (void*)0                          // array buffer offset
                              );
        
        // 3rd attribute buffer : normals
        GLint normal_attribute = glGetAttribLocation(shaderProgram, "vertexNormal_modelspace");
        glEnableVertexAttribArray(normal_attribute);
        glBindBuffer(GL_ARRAY_BUFFER, normalsbuffer);
        glVertexAttribPointer(
                              normal_attribute,                                // attribute
                              3,                                // size
                              GL_FLOAT,                         // type
                              GL_FALSE,                         // normalized?
                              0,                                // stride
                              (void*)0                          // array buffer offset
                              );
        // Model matrix : an identity matrix (model will be at the origin)
        Model = glm::mat4(1.0f); // Changes for each model !
        Model = glm::translate(Model, glm::vec3(x, y, z));
        Model = glm::scale(Model, glm::vec3(0.2f,0.2f,0.2f));
        
        myShader->setModelMatrix(Model);
        myShader->SetUniform("M", Model);
        
        
        glm::mat4 MVP = myShader->Projection * myShader->View * Model;
        glm::mat4 biasMatrix(
                             0.5, 0.0, 0.0, 0.0,
                             0.0, 0.5, 0.0, 0.0,
                             0.0, 0.0, 0.5, 0.0,
                             0.5, 0.5, 0.5, 1.0
                             );
        glm::mat4 depthBiasMVP = biasMatrix * depthMVP;
        // Send our transformation to the currently bound shader,
        // in the "MVP" uniform
        myShader->SetUniform("M", Model);
        myShader->SetUniform("MVP", MVP);
        myShader->SetUniform("DepthBiasMVP", depthBiasMVP);
        
        myShader->SetUniformMVP2();
        glDrawArrays(GL_TRIANGLES, 0, this->numberOfPoints());
        
        glDisableVertexAttribArray(position_attribute);
        glDisableVertexAttribArray(color_attribute);
        glDisableVertexAttribArray(normal_attribute);
        
    }
    
    int mostra=0;
    
    int sizeOfAllFloats(){
        return m_nx*m_ny*6*3*sizeof(GLfloat); //10 vezes 10 sub quadrados com dois triangulos. Cada triangulo 3 vertices. Cada Vertice sao 3 floats
    }
    
    int Index (int i, int j) {
        return i + j*(m_ny);
    }
    
    int numberOfPoints(){
        return m_nx*m_ny*2*3;
    }
    
    void setPosition(glm::vec3 pos){
        x=pos.x;
        y=pos.y;
        z=pos.z;
    }
};

/*
 Copyright 2011 Etay Meiri
 
 This program is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.
 
 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.
 
 You should have received a copy of the GNU General Public License
 along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
class GBuffer{
public:
    
    enum GBUFFER_TEXTURE_TYPE {
        GBUFFER_TEXTURE_TYPE_POSITION,
        GBUFFER_TEXTURE_TYPE_DIFFUSE,
        GBUFFER_TEXTURE_TYPE_NORMAL,
        GBUFFER_TEXTURE_TYPE_TEXCOORD,
        GBUFFER_NUM_TEXTURES
    };
    
    GBuffer(){
        m_fbo = 0;
        m_depthTexture = 0;
        ZERO_MEM(m_textures);
    }
    
    ~GBuffer(){
        if (m_fbo != 0) {
            glDeleteFramebuffers(1, &m_fbo);
        }
        
        if (m_textures[0] != 0) {
            glDeleteTextures(ARRAY_SIZE_IN_ELEMENTS(m_textures), m_textures);
        }
        
        if (m_depthTexture != 0) {
            glDeleteTextures(1, &m_depthTexture);
        }
    }
    
    bool Init(unsigned int WindowWidth, unsigned int WindowHeight){
        // Create the FBO
        glGenFramebuffers(1, &m_fbo);
        glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);
        
        // Create the gbuffer textures
        glGenTextures(ARRAY_SIZE_IN_ELEMENTS(m_textures), m_textures);
        glGenTextures(1, &m_depthTexture);
        
        for (unsigned int i = 0 ; i < ARRAY_SIZE_IN_ELEMENTS(m_textures) ; i++) {
            glBindTexture(GL_TEXTURE_2D, m_textures[i]);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB32F, WindowWidth, WindowHeight, 0, GL_RGB, GL_FLOAT, NULL);
            glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, GL_TEXTURE_2D, m_textures[i], 0);
        }
        
        // depth
        glBindTexture(GL_TEXTURE_2D, m_depthTexture);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT32F, WindowWidth, WindowHeight, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
        glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, m_depthTexture, 0);
        
        GLenum DrawBuffers[] = { GL_COLOR_ATTACHMENT0,
            GL_COLOR_ATTACHMENT1,
            GL_COLOR_ATTACHMENT2,
            GL_COLOR_ATTACHMENT3 };
        
        glDrawBuffers(ARRAY_SIZE_IN_ELEMENTS(DrawBuffers), DrawBuffers);
        
        GLenum Status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
        
        if (Status != GL_FRAMEBUFFER_COMPLETE) {
            printf("FB error, status: 0x%x\n", Status);
            return false;
        }
        
        // restore default FBO
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
        
        return true;
    }
    
    void BindForWriting(){
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_fbo);
    }
    
    void BindForReading(){
        glBindFramebuffer(GL_READ_FRAMEBUFFER, m_fbo);
    }
    void SetReadBuffer(GBUFFER_TEXTURE_TYPE TextureType){
        glReadBuffer(GL_COLOR_ATTACHMENT0 + TextureType);
    }
    
    //private:
    
    GLuint m_fbo;
    GLuint m_textures[GBUFFER_NUM_TEXTURES];
    GLuint m_depthTexture;
};


class FBO{
public:
//    GLuint m_fbo;
    GLuint m_textures[3];
    GLuint m_depthTexture;
    
    GLuint FramebufferName;
    //    GLuint renderedTexture;
    GLuint depthrenderbuffer;
    
    FBO(){
//        m_fbo = 0;
        m_depthTexture = 0;
        ZERO_MEM(m_textures);
        
    }
    bool Init(unsigned int WindowWidth, unsigned int WindowHeight){
        
        // The framebuffer, which regroups 0, 1, or more textures, and 0 or 1 depth buffer.
        FramebufferName = 0;
        glGenFramebuffers(1, &FramebufferName);
        glBindFramebuffer(GL_FRAMEBUFFER, FramebufferName);
        
        // Create the gbuffer textures
        for (unsigned int i = 0 ; i < ARRAY_SIZE_IN_ELEMENTS(m_textures) ; i++) {
            glGenTextures(1, &m_textures[i]);
            glBindTexture(GL_TEXTURE_2D, m_textures[i]);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB32F, WindowWidth, WindowHeight, 0, GL_RGB, GL_FLOAT, NULL);
            glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0+i, GL_TEXTURE_2D, m_textures[i], 0);
            // Poor filtering
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        }
        
        // The depth buffer
        glGenRenderbuffers(1, &depthrenderbuffer);
        glBindRenderbuffer(GL_RENDERBUFFER, depthrenderbuffer);
        glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, 800, 800);
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depthrenderbuffer);
        
        // Set the list of draw buffers.
        GLenum DrawBuffers[] = { GL_COLOR_ATTACHMENT0,
            GL_COLOR_ATTACHMENT1,
            GL_COLOR_ATTACHMENT2 };
        
        glDrawBuffers(ARRAY_SIZE_IN_ELEMENTS(DrawBuffers), DrawBuffers);
        
        // Always check that our framebuffer is ok
        if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
            return false;
        return true;
    }
};

Luz * luzes[50];

class DeferRender{
public:
    Shader * quadShader;
    Shader * lightingShader;
    
    GLuint texID;
    GLuint quad_vertexbuffer;
    
    GLuint posTex, colorTex, nomalTex;
    
    DeferRender(){
        GLfloat g_quad_vertex_buffer_data[] = {
            -1.0f, -1.0f, 0.0f,
            1.0f, -1.0f, 0.0f,
            -1.0f,  1.0f, 0.0f,
            -1.0f,  1.0f, 0.0f,
            1.0f, -1.0f, 0.0f,
            1.0f,  1.0f, 0.0f,
        };
        
        glGenBuffers(1, &quad_vertexbuffer);
        glBindBuffer(GL_ARRAY_BUFFER, quad_vertexbuffer);
        glBufferData(GL_ARRAY_BUFFER, sizeof(g_quad_vertex_buffer_data), g_quad_vertex_buffer_data, GL_STATIC_DRAW);
        
        // Create and compile our GLSL program from the shaders
        quadShader = new Shader("shaders6/ex-deferVertex.cpp", "shaders6/ex-deferFrag.cpp");
        lightingShader = new Shader("shaders6/deferVertex.cpp", "shaders6/deferFrag.cpp");
        
        texID = glGetUniformLocation(quadShader->programID(), "renderedTexture");
    }
    ~DeferRender(){
        
    }
    
    void setRenderToFrameBuffer(GLuint FramebufferName){
        glBindFramebuffer(GL_FRAMEBUFFER, FramebufferName);
    }
    void setRenderToScreen(){
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }
    
    void showTexture(GLuint renderedTexture){
        setRenderToScreen();
		glViewport(0,0,800,800); // Render on the whole framebuffer, complete from the lower left corner to the upper right
        
		// Clear the screen
		glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        
		// Bind our texture in Texture Unit 0
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, renderedTexture);
        //		glBindTexture(GL_TEXTURE_2D, depthTexture);
        
		// Set our "renderedTexture" sampler to user Texture Unit 0
		//glUniform1i(this->texID, 4);
        
		// 1rst attribute buffer : vertices
		glEnableVertexAttribArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, this->quad_vertexbuffer);
		glVertexAttribPointer(
                              0,                  // attribute 0. No particular reason for 0, but must match the layout in the shader.
                              3,                  // size
                              GL_FLOAT,           // type
                              GL_FALSE,           // normalized?
                              0,                  // stride
                              (void*)0            // array buffer offset
                              );
        
        // Use our shader
		glUseProgram(this->quadShader->programID());
		// Draw the triangles !
		glDrawArrays(GL_TRIANGLES, 0, 6); // 2*3 indices starting at 0 -> 2 triangles
        
		glDisableVertexAttribArray(0);
    }
    
    void render(FBO * fbo){
        posTex = glGetUniformLocation(lightingShader->programID(), "posTex");
        colorTex = glGetUniformLocation(lightingShader->programID(), "corTex");
        nomalTex = glGetUniformLocation(lightingShader->programID(), "normalTex");
        
        setRenderToScreen();
		glViewport(0,0,800,800); // Render on the whole framebuffer, complete from the lower left corner to the upper right
        
		// Clear the screen
		glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        
        // Use our shader
		glUseProgram(this->lightingShader->programID());
        
		// Bind our texture in Texture Unit 0
        glActiveTexture(GL_TEXTURE0);
        glEnable(GL_TEXTURE_2D);
        glBindTexture(GL_TEXTURE_2D, fbo->m_textures[0]);
        glUniform1i ( posTex, 0 );
        
        glActiveTexture(GL_TEXTURE1);
        glEnable(GL_TEXTURE_2D);
        glBindTexture(GL_TEXTURE_2D, fbo->m_textures[1]);
        glUniform1i ( colorTex, 1 );
        
        glActiveTexture(GL_TEXTURE2);
        glEnable(GL_TEXTURE_2D);
        glBindTexture(GL_TEXTURE_2D, fbo->m_textures[2]);
        glUniform1i ( nomalTex, 2 );
        
		// 1rst attribute buffer : vertices
		glEnableVertexAttribArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, this->quad_vertexbuffer);
		glVertexAttribPointer(
                              0,                  // attribute 0. No particular reason for 0, but must match the layout in the shader.
                              3,                  // size
                              GL_FLOAT,           // type
                              GL_FALSE,           // normalized?
                              0,                  // stride
                              (void*)0            // array buffer offset
                              );
        
        
        
        GLfloat lights[50*3];
        GLfloat lightColors[50*3];
        //passa as posicoes das luzes pra float
        for (int i=0;i<50;i++){
            lights[i*3 +0]=luzes[i]->pos.x;
            lights[i*3 +1]=luzes[i]->pos.y;
            lights[i*3 +2]=luzes[i]->pos.z;
            
            lightColors[i*3 +0] = luzes[i]->cor.r;
            lightColors[i*3 +1] = luzes[i]->cor.g;
            lightColors[i*3 +2] = luzes[i]->cor.b;
        }
        
        glUniform1fv(glGetUniformLocation(lightingShader->programID(), "lightPos"), 50*3, lights);
        glUniform1fv(glGetUniformLocation(lightingShader->programID(), "lightCor"), 50*3, lightColors);
        //        lightingShader->SetUniform("lightPos", luzes[0]->pos.x, luzes[0]->pos.y, luzes[0]->pos.z);
        
		// Draw the triangles !
		glDrawArrays(GL_TRIANGLES, 0, 6); // 2*3 indices starting at 0 -> 2 triangles
        
		glDisableVertexAttribArray(0);
    }
};

class ShadowMap{
public:
    GLuint FramebufferName;
    GLuint depthTexture;
    
    ShadowMap(){
        FramebufferName = 0;
    }
    
    bool Init(unsigned int WindowWidth, unsigned int WindowHeight){
        // The framebuffer, which regroups 0, 1, or more textures, and 0 or 1 depth buffer.
        glGenFramebuffers(1, &FramebufferName);
        glBindFramebuffer(GL_FRAMEBUFFER, FramebufferName);
        
        // Depth texture. Slower than a depth buffer, but you can sample it later in your shader
        glGenTextures(1, &depthTexture);
        glBindTexture(GL_TEXTURE_2D, depthTexture);
        glTexImage2D(GL_TEXTURE_2D, 0,GL_DEPTH_COMPONENT16, WindowWidth, WindowHeight, 0,GL_DEPTH_COMPONENT, GL_FLOAT, 0);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_R_TO_TEXTURE);
        
        glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, depthTexture, 0);
        
        // No color output in the bound framebuffer, only depth.
        glDrawBuffer(GL_NONE);
        
        // Always check that our framebuffer is ok
        if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
            return false;
        return true;

    }
};

// ========== Prototipos funcoes ==========
void GLFWCALL window_resized(int width, int height);
void keyboard(int key, int action);
void configuraContexto();
void configuraCena();
void displayCena();
void updateLuz();

// ========== Variaveis globais ==========
Shader * myShader;
Sphere * s[10][10];
Grid *g;
Sphere * luz;

// ========== Testes ====================
GBuffer m_gbuffer;

void DSLightPass()
{
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
    
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    m_gbuffer.BindForReading();
    
    GLint HalfWidth = (GLint)(WINDOW_WIDTH / 2.0f);
    GLint HalfHeight = (GLint)(WINDOW_HEIGHT / 2.0f);
    
    m_gbuffer.SetReadBuffer(GBuffer::GBUFFER_TEXTURE_TYPE_POSITION);
    glBlitFramebuffer(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT, 0, 0, HalfWidth, HalfHeight, GL_COLOR_BUFFER_BIT, GL_LINEAR);
    
    m_gbuffer.SetReadBuffer(GBuffer::GBUFFER_TEXTURE_TYPE_DIFFUSE);
    glBlitFramebuffer(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT, 0, HalfHeight, HalfWidth, WINDOW_HEIGHT, GL_COLOR_BUFFER_BIT, GL_LINEAR);
    
    m_gbuffer.SetReadBuffer(GBuffer::GBUFFER_TEXTURE_TYPE_NORMAL);
    glBlitFramebuffer(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT, HalfWidth, HalfHeight, WINDOW_WIDTH, WINDOW_HEIGHT, GL_COLOR_BUFFER_BIT, GL_LINEAR);
    
    m_gbuffer.SetReadBuffer(GBuffer::GBUFFER_TEXTURE_TYPE_TEXCOORD);
    glBlitFramebuffer(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT, HalfWidth, 0, WINDOW_WIDTH, HalfHeight, GL_COLOR_BUFFER_BIT, GL_LINEAR);
}


// ========== Main ==========
int main2( void );
int main3( void );

int main () {

    configuraContexto();

//    main3();
//    return 0;
    
    // Create a vertex array object
    GLuint VertexArrayID;
    // Use a Vertex Array Object
    glGenVertexArrays(1, &VertexArrayID);
    glBindVertexArray(VertexArrayID);
    
    configuraCena();

    main2();
    return 0;
    
    
    FBO * fbo = new FBO();
    fbo->Init(WINDOW_WIDTH, WINDOW_HEIGHT);
    DeferRender * dr = new DeferRender();
    
    dr->setRenderToFrameBuffer(fbo->FramebufferName);
    displayCena();
    
	// Create a rendering loop
	int running = GL_TRUE;
    
	while(running) {
        updateLuz();
        
        //		dr->showTexture(fbo->m_textures[2]);
		dr->render(fbo);
        
        glfwSwapBuffers();
		glfwPollEvents();
		running = glfwGetWindowParam(GLFW_OPENED);
	}
    
    // Cleanup VBO and shader
	glDeleteProgram(myShader->programID());
	glDeleteVertexArrays(1, &VertexArrayID);
    
	// Terminate GLFW
	glfwTerminate();
	return 0;
}

glm::vec3 randPos(){
    return glm::vec3(rand()%20 -10, rand()%20-10, rand()%20 -10);
}
glm::vec3 randCor(){
    return glm::vec3((rand()%10+1)/10.0, (rand()%10+1)/10.0, (rand()%10+1)/10.0);
}

void configuraCena(){
    c.x =0;
    c.y =4;
    c.z =8;
    
//    srand (time(NULL));
    
    //inicializa o primeiro shader
//    myShader = new Shader("shadowmapShaders/DepthRTT.vertexshader",
//                          "shadowmapShaders/DepthRTT.fragmentshader");
//    
    //inicializa os objetos
    for (int i=0;i<10;i++){
        for (int j=0; j<10; j++) {
            s[i][j] = new Sphere(64,64);
            s[i][j]->setPositionXZ(i,j);
        }
    }
    g = new Grid(64,64);
    luz = new Sphere(32,32);
    
//    luzes[0] = new Luz(glm::vec3(0,4,8),glm::vec3(1));
    //inicializa as luzes
//    for (int i=0;i<50;i++){
//        glm::vec3 c =randCor();
//        //printf("%.2f,%.2f,%.2f\n",c.r,c.g,c.b);
//        luzes[i] = new Luz(randPos(),c);
//    }
    
}

void configuraContexto(){
    // Initialize GLFW
	if ( !glfwInit()) {
		std::cerr << "Failed to initialize GLFW! I'm out!" << std::endl;
		exit(-1);
	}
    
	// Use OpenGL 3.2 core profile
	glfwOpenWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwOpenWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
	glfwOpenWindowHint(GLFW_OPENGL_VERSION_MAJOR, 3);
	glfwOpenWindowHint(GLFW_OPENGL_VERSION_MINOR, 2);
    
	// Open a window and attach an OpenGL rendering context to the window surface
	if( !glfwOpenWindow(1024, 1024, 8, 8, 8, 8, 24, 8, GLFW_WINDOW)) {
		std::cerr << "Failed to open a window! I'm out!" << std::endl;
		glfwTerminate();
		exit(-1);
	}
    
	// Print the OpenGL version
	int major, minor, rev;
	glfwGetGLVersion(&major, &minor, &rev);
	std::cout << "OpenGL - " << major << "." << minor << "." << rev << std::endl;
    
	// Initialize GLEW
	glewExperimental = GL_TRUE;
	if(glewInit() != GLEW_OK) {
		std::cerr << "Failed to initialize GLEW! I'm out!" << std::endl;
		glfwTerminate();
		exit(-1);
	}
    
    // Dark blue background
//	glClearColor(0.0f, 0.0f, 0.4f, 0.0f);
//    // Enable depth test
    glEnable(GL_DEPTH_TEST);
//
//    glEnable(GL_CULL_FACE);
    
	// Register a callback function for window resize events
	glfwSetWindowSizeCallback( window_resized );
    // Register a callback function for keyboard pressed events
	glfwSetKeyCallback(keyboard);
}

void updateLuz(){
    luz->setPosition(glm::vec3(c.x,c.y,c.z));
    luz->draw(myShader);
    glm::vec3 cam = glm::vec3(c.x,c.y,c.z);
    cam = glm::rotateY(cam, 0.5f);
    c.x = cam.x;
    c.y = cam.y;
    c.z = cam.z;
    
    //inicializa as luzes
    glm::vec3 aux;
    for (int i=0;i<50;i++){
        aux = glm::rotateY(luzes[i]->pos, 0.5f);
        luzes[i]->setPos(aux);
    }
    
}

void displayCena(){
    // Clear the screen
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    for (int i=0;i<10;i++){
        for (int j=0; j<10; j++) {
            s[i][j]->draw(myShader);
        }
    }
    g->draw(myShader);
}

// Called when the window is resized
void GLFWCALL window_resized(int width, int height) {
	// Use red to clear the screen
	glClearColor(0, 0, 0, 1);
    
	// Set the viewport
	glViewport(0, 0, width, height);
    
	glClear(GL_COLOR_BUFFER_BIT);
    
}

// Called for keyboard events
void keyboard(int key, int action) {
	if( key == 'Q' && action == GLFW_PRESS) {
		glfwTerminate();
		exit(0);
	}
}


int main2( void ){
    
	// Dark blue background
	glClearColor(1.0f, 1.0f, 1.0f, 0.0f);
	// Enable depth test
	glEnable(GL_DEPTH_TEST);
	// Accept fragment if it closer to the camera than the former one
	glDepthFunc(GL_LESS);
	// Cull triangles which normal is not towards the camera
	glEnable(GL_CULL_FACE);
    
//	GLuint VertexArrayID;
//	glGenVertexArrays(1, &VertexArrayID);
//	glBindVertexArray(VertexArrayID);
    
    //============================================================
    
	ShadowMap * shadowMap = new ShadowMap();
    shadowMap->Init(1024, 1024);
    
    //============================================================
    Shader * depthShader = new Shader("shadowmapShaders/DepthRTT.vertexshader",
                                      "shadowmapShaders/DepthRTT.fragmentshader");
    
    glm::vec3 lightInvDir = glm::vec3(5,5,0);
    // Compute the MVP matrix from the light's point of view
    glm::mat4 depthProjectionMatrix = glm::ortho<float>(-5,5,-5,5,-10,10);
    glm::mat4 depthViewMatrix = glm::lookAt(lightInvDir, glm::vec3(0,0,0), glm::vec3(0,1,0));
    depthShader->setProjectionMatrix(depthProjectionMatrix);
    depthShader->setViewMatrix(depthViewMatrix);
    
    //============================================================
//    Shader * shadowShader = new Shader("shadowmapShaders/ShadowMapping_SimpleVersion.vertexshader",
//                                      "shadowmapShaders/ShadowMapping_SimpleVersion.fragmentshader");
    Shader * shadowShader = new Shader("shadowmapShaders/ShadowMapping.vertexshader",
                                       "shadowmapShaders/ShadowMapping.fragmentshader");
    glm::mat4 ProjectionMatrix = glm::perspective(45.0f, 1.0f/*4.0f / 3.0f*/, 1.0f, 30.0f);
    glm::mat4 ViewMatrix = glm::lookAt(
                                       glm::vec3(0,4,8), // Camera is at (x,y,z)
                                       glm::vec3(0,0,0), // and looks at the origin
                                       glm::vec3(0,1,0) // Head is up (set to 0,-1,0 to look upside-down)
                                       );
    shadowShader->setProjectionMatrix(ProjectionMatrix);
    shadowShader->setViewMatrix(ViewMatrix);
    shadowShader->SetUniform("LightInvDirection_worldspace", lightInvDir.x, lightInvDir.y, lightInvDir.z);
    shadowShader->SetUniform("V", ViewMatrix);
    
    //============================================================
    
    Shader * quadShader;
    GLfloat g_quad_vertex_buffer_data[] = {
        -1.0f, -1.0f, 0.0f,
        1.0f, -1.0f, 0.0f,
        -1.0f,  1.0f, 0.0f,
        -1.0f,  1.0f, 0.0f,
        1.0f, -1.0f, 0.0f,
        1.0f,  1.0f, 0.0f,
    };
    GLuint texID;
    GLuint quad_vertexbuffer;
    glGenBuffers(1, &quad_vertexbuffer);
    glBindBuffer(GL_ARRAY_BUFFER, quad_vertexbuffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(g_quad_vertex_buffer_data), g_quad_vertex_buffer_data, GL_STATIC_DRAW);
    
    // Create and compile our GLSL program from the shaders
    quadShader = new Shader("shadowmapShaders/Passthrough.vertexshader", "shadowmapShaders/SimpleTexture.fragmentshader");
    texID = glGetUniformLocation(quadShader->programID(), "mytexture");
    
    //============================================================
    
    // Create a rendering loop
	int running = GL_TRUE;
	while(running) {
        
        glBindTexture(GL_TEXTURE_2D, shadowMap->depthTexture);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_R_TO_TEXTURE);
        
		// Render to our framebuffer
		glBindFramebuffer(GL_FRAMEBUFFER, shadowMap->FramebufferName);
		glViewport(0,0,1024,1024); // Render on the whole framebuffer, complete from the lower left corner to the upper right
        
		// We don't use bias in the shader, but instead we draw back faces,
		// which are already separated from the front faces by a small distance
		// (if your geometry is made this way)
		glEnable(GL_CULL_FACE);
		glCullFace(GL_BACK); // Cull back-facing triangles -> draw only front-facing triangles
        
		// Clear the screen
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        
        for (int i=0;i<10;i++){
            for (int j=0; j<10; j++) {
                s[i][j]->drawDepthShader(depthShader);
            }
        }
        g->drawDepthShader(depthShader);
        
        
        //============================================================
		// Render to the screen
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glViewport(0,0,1024,1024); // Render on the whole framebuffer, complete from the lower left corner to the upper right
        
		glEnable(GL_CULL_FACE);
		glCullFace(GL_BACK); // Cull back-facing triangles -> draw only front-facing triangles
        
		// Clear the screen
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        
        for (int i=0;i<10;i++){
            for (int j=0; j<10; j++) {
                s[i][j]->drawShadowShader(shadowShader,shadowMap->depthTexture);
            }
        }
        g->drawShadowShader(shadowShader,shadowMap->depthTexture);
        
        //============================================================

        // Render Shadow Map
        // Render only on a corner of the window (or we we won't see the real rendering...)
		glViewport(0,0,128,128);
		// Use our shader
		glUseProgram(quadShader->programID());
        
		// Bind our texture in Texture Unit 0
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, shadowMap->depthTexture);
        glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_NONE );
        //        glTexParameteri( GL_TEXTURE_2D, GL_DEPTH_TEXTURE_MODE, GL_LUMINANCE );
		// Set our "renderedTexture" sampler to user Texture Unit 0
		glUniform1i(texID, 0);
        
		// 1rst attribute buffer : vertices
		glEnableVertexAttribArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, quad_vertexbuffer);
		glVertexAttribPointer(
                              0,                  // attribute 0. No particular reason for 0, but must match the layout in the shader.
                              3,                  // size
                              GL_FLOAT,           // type
                              GL_FALSE,           // normalized?
                              0,                  // stride
                              (void*)0            // array buffer offset
                              );
        
		// Draw the triangle !
		// You have to disable GL_COMPARE_R_TO_TEXTURE above in order to see anything !
        glDrawArrays(GL_TRIANGLES, 0, 6); // 2*3 indices starting at 0 -> 2 triangles
		glDisableVertexAttribArray(0);
        //============================================================
        
		// Swap buffers
		glfwSwapBuffers();
		glfwPollEvents();
		running = glfwGetWindowParam(GLFW_OPENED);
	}
    
	// Cleanup VBO and shader
	glDeleteProgram(shadowShader->programID());
	glDeleteProgram(depthShader->programID());
	glDeleteTextures(1, &shadowMap->depthTexture);
    
	glDeleteFramebuffers(1, &shadowMap->FramebufferName);
	glDeleteTextures(1, &shadowMap->depthTexture);
    
    
	// Close OpenGL window and terminate GLFW
	glfwTerminate();
    
	return 0;
}

int main3( void )
{
	// Dark blue background
	glClearColor(0.0f, 0.0f, 0.4f, 0.0f);
    
	// Enable depth test
	glEnable(GL_DEPTH_TEST);
    
	// Accept fragment if it closer to the camera than the former one
	glDepthFunc(GL_LESS);
    
	// Cull triangles which normal is not towards the camera
	glEnable(GL_CULL_FACE);
    
	GLuint VertexArrayID;
	glGenVertexArrays(1, &VertexArrayID);
	glBindVertexArray(VertexArrayID);
    
	// Create and compile our GLSL program from the shaders
	GLuint depthProgramID = LoadShaders( "shadowmapShaders/DepthRTT.vertexshader", "shadowmapShaders/DepthRTT.fragmentshader" );
    
	// Get a handle for our "MVP" uniform
	GLuint depthMatrixID = glGetUniformLocation(depthProgramID, "depthMVP");
    
	// Load the texture
	GLuint Texture = loadDDS("shadowmapShaders/uvmap.DDS");
	
	// Read our .obj file
	std::vector<glm::vec3> vertices;
	std::vector<glm::vec2> uvs;
	std::vector<glm::vec3> normals;
	bool res = loadOBJ("shadowmapShaders/room_thickwalls.obj", vertices, uvs, normals);
//	bool res = loadOBJ("shadowmapShaders/diststation.obj", vertices, uvs, normals);
    
	std::vector<unsigned short> indices;
	std::vector<glm::vec3> indexed_vertices;
	std::vector<glm::vec2> indexed_uvs;
	std::vector<glm::vec3> indexed_normals;
	indexVBO(vertices, uvs, normals, indices, indexed_vertices, indexed_uvs, indexed_normals);
    
	// Load it into a VBO
    
	GLuint vertexbuffer;
	glGenBuffers(1, &vertexbuffer);
	glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
	glBufferData(GL_ARRAY_BUFFER, indexed_vertices.size() * sizeof(glm::vec3), &indexed_vertices[0], GL_STATIC_DRAW);
    
	GLuint uvbuffer;
	glGenBuffers(1, &uvbuffer);
	glBindBuffer(GL_ARRAY_BUFFER, uvbuffer);
	glBufferData(GL_ARRAY_BUFFER, indexed_uvs.size() * sizeof(glm::vec2), &indexed_uvs[0], GL_STATIC_DRAW);
    
	GLuint normalbuffer;
	glGenBuffers(1, &normalbuffer);
	glBindBuffer(GL_ARRAY_BUFFER, normalbuffer);
	glBufferData(GL_ARRAY_BUFFER, indexed_normals.size() * sizeof(glm::vec3), &indexed_normals[0], GL_STATIC_DRAW);
    
	// Generate a buffer for the indices as well
	GLuint elementbuffer;
	glGenBuffers(1, &elementbuffer);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, elementbuffer);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned short), &indices[0], GL_STATIC_DRAW);
    
    
	// ---------------------------------------------
	// Render to Texture - specific code begins here
	// ---------------------------------------------
    
	// The framebuffer, which regroups 0, 1, or more textures, and 0 or 1 depth buffer.
	GLuint FramebufferName = 0;
	glGenFramebuffers(1, &FramebufferName);
	glBindFramebuffer(GL_FRAMEBUFFER, FramebufferName);
    
	// Depth texture. Slower than a depth buffer, but you can sample it later in your shader
	GLuint depthTexture;
	glGenTextures(1, &depthTexture);
	glBindTexture(GL_TEXTURE_2D, depthTexture);
	glTexImage2D(GL_TEXTURE_2D, 0,GL_DEPTH_COMPONENT16, 1024, 1024, 0,GL_DEPTH_COMPONENT, GL_FLOAT, 0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_R_TO_TEXTURE);
    
	glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, depthTexture, 0);
    
	// No color output in the bound framebuffer, only depth.
	glDrawBuffer(GL_NONE);
    
	// Always check that our framebuffer is ok
	if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		return false;
    
	
	// The quad's FBO. Used only for visualizing the shadowmap.
	static const GLfloat g_quad_vertex_buffer_data[] = {
		-1.0f, -1.0f, 0.0f,
        1.0f, -1.0f, 0.0f,
		-1.0f,  1.0f, 0.0f,
		-1.0f,  1.0f, 0.0f,
        1.0f, -1.0f, 0.0f,
        1.0f,  1.0f, 0.0f,
	};
    
	GLuint quad_vertexbuffer;
	glGenBuffers(1, &quad_vertexbuffer);
	glBindBuffer(GL_ARRAY_BUFFER, quad_vertexbuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(g_quad_vertex_buffer_data), g_quad_vertex_buffer_data, GL_STATIC_DRAW);
    
	// Create and compile our GLSL program from the shaders
	GLuint quad_programID = LoadShaders( "shadowmapShaders/Passthrough.vertexshader", "shadowmapShaders/SimpleTexture.fragmentshader" );
	GLuint texID = glGetUniformLocation(quad_programID, "mytexture");
    
    
	// Create and compile our GLSL program from the shaders
	GLuint programID = LoadShaders( "shadowmapShaders/ShadowMapping.vertexshader", "shadowmapShaders/ShadowMapping.fragmentshader" );
    
	// Get a handle for our "myTextureSampler" uniform
	GLuint TextureID  = glGetUniformLocation(programID, "myTextureSampler");
    
	// Get a handle for our "MVP" uniform
	GLuint MatrixID = glGetUniformLocation(programID, "MVP");
	GLuint ViewMatrixID = glGetUniformLocation(programID, "V");
	GLuint ModelMatrixID = glGetUniformLocation(programID, "M");
	GLuint DepthBiasID = glGetUniformLocation(programID, "DepthBiasMVP");
	GLuint ShadowMapID = glGetUniformLocation(programID, "shadowMap");
	
	// Get a handle for our "LightPosition" uniform
	GLuint lightInvDirID = glGetUniformLocation(programID, "LightInvDirection_worldspace");
    
    // Create a rendering loop
	int running = GL_TRUE;
	while(running) {
        
		// Render to our framebuffer
		glBindFramebuffer(GL_FRAMEBUFFER, FramebufferName);
		glViewport(0,0,1024,1024); // Render on the whole framebuffer, complete from the lower left corner to the upper right
        
		// We don't use bias in the shader, but instead we draw back faces,
		// which are already separated from the front faces by a small distance
		// (if your geometry is made this way)
//		glEnable(GL_CULL_FACE);
        glDisable(GL_CULL_FACE);
//		glCullFace(GL_BACK); // Cull back-facing triangles -> draw only front-facing triangles
        
		// Clear the screen
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        
		// Use our shader
		glUseProgram(depthProgramID);
        
		glm::vec3 lightInvDir = glm::vec3(0.5f,2,2);
        
		// Compute the MVP matrix from the light's point of view
		glm::mat4 depthProjectionMatrix = glm::ortho<float>(-10,10,-10,10,-10,20);
//        glm::mat4 depthProjectionMatrix = glm::ortho<float>(-5000,5000,-5000,5000,-5000,5000);
//        glm::mat4 depthProjectionMatrix = glm::ortho<float>(230822,239822,-1000,1000,-20000,-30500);//234822, 300, -26500
		glm::mat4 depthViewMatrix = glm::lookAt(lightInvDir, glm::vec3(0,0,0), glm::vec3(0,1,0));
		// or, for spot light :
		//glm::vec3 lightPos(5, 20, 20);
		//glm::mat4 depthProjectionMatrix = glm::perspective<float>(45.0f, 1.0f, 2.0f, 50.0f);
		//glm::mat4 depthViewMatrix = glm::lookAt(lightPos, lightPos-lightInvDir, glm::vec3(0,1,0));
        
		glm::mat4 depthModelMatrix = glm::mat4(1.0);
		glm::mat4 depthMVP = depthProjectionMatrix * depthViewMatrix * depthModelMatrix;
        
		// Send our transformation to the currently bound shader,
		// in the "MVP" uniform
		glUniformMatrix4fv(depthMatrixID, 1, GL_FALSE, &depthMVP[0][0]);
        
		// 1rst attribute buffer : vertices
		glEnableVertexAttribArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
		glVertexAttribPointer(
                              0,  // The attribute we want to configure
                              3,                  // size
                              GL_FLOAT,           // type
                              GL_FALSE,           // normalized?
                              0,                  // stride
                              (void*)0            // array buffer offset
                              );
        
		// Index buffer
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, elementbuffer);
        
		// Draw the triangles !
		glDrawElements(
                       GL_TRIANGLES,      // mode
                       indices.size(),    // count
                       GL_UNSIGNED_SHORT, // type
                       (void*)0           // element array buffer offset
                       );
        
		glDisableVertexAttribArray(0);
        
        
        
		// Render to the screen
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glViewport(0,0,1024,1024); // Render on the whole framebuffer, complete from the lower left corner to the upper right
        
		glEnable(GL_CULL_FACE);
//        glDisable(GL_CULL_FACE);
		glCullFace(GL_BACK); // Cull back-facing triangles -> draw only front-facing triangles
        
		// Clear the screen
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        
		// Use our shader
		glUseProgram(programID);
        
        glm::mat4 ProjectionMatrix = glm::perspective(45.0f, 1.0f/*4.0f / 3.0f*/, 1.0f, 50.0f);
        glm::mat4 ViewMatrix = glm::lookAt(
//                                           glm::vec3(14,6,4), // Camera is at (x,y,z)
                                           glm::vec3(20,10,-5), // Camera is at (x,y,z)
                                           glm::vec3(0,4,0), // and looks at the origin
                                           glm::vec3(0,1,0) // Head is up (set to 0,-1,0 to look upside-down)
                                           );
        
//        glm::mat4 ProjectionMatrix = glm::perspective(45.0f, 1.0f/*4.0f / 3.0f*/, 10.0f, 700.0f);
//		glm::mat4 ViewMatrix = glm::lookAt(
//                                           //                                           glm::vec3(14,6,4), // Camera is at (x,y,z)
//                                           glm::vec3(300,300,300), // Camera is at (x,y,z)
//                                           glm::vec3(100,200,100), // and looks at the origin
//                                           glm::vec3(0,1,0) // Head is up (set to 0,-1,0 to look upside-down)
//                                           );
//        glm::mat4 ProjectionMatrix = glm::perspective(45.0f, 1.0f/*4.0f / 3.0f*/, 100.0f, 10000.0f);
//		glm::mat4 ViewMatrix = glm::lookAt(
//                                           //                                           glm::vec3(14,6,4), // Camera is at (x,y,z)
//                                           glm::vec3(238822, 907, -26800), // Camera is at (x,y,z)
//                                           glm::vec3(234822, 900, -26800), // and looks at the origin
//                                           glm::vec3(0,1,0) // Head is up (set to 0,-1,0 to look upside-down)
//                                           );

        
//        glm::mat4 ProjectionMatrix = glm::perspective(45.0f, 1.0f/*4.0f / 3.0f*/, 10.0f, 700.0f);
//		glm::mat4 ViewMatrix = glm::lookAt(glm::vec3(14,6,4), glm::vec3(0,1,0), glm::vec3(0,1,0));
		glm::mat4 ModelMatrix = glm::mat4(1.0);
		glm::mat4 MVP = ProjectionMatrix * ViewMatrix * ModelMatrix;
		
		glm::mat4 biasMatrix(
                             0.5, 0.0, 0.0, 0.0,
                             0.0, 0.5, 0.0, 0.0,
                             0.0, 0.0, 0.5, 0.0,
                             0.5, 0.5, 0.5, 1.0
                             );
        
		glm::mat4 depthBiasMVP = biasMatrix*depthMVP;
        
		// Send our transformation to the currently bound shader,
		// in the "MVP" uniform
		glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &MVP[0][0]);
		glUniformMatrix4fv(ModelMatrixID, 1, GL_FALSE, &ModelMatrix[0][0]);
		glUniformMatrix4fv(ViewMatrixID, 1, GL_FALSE, &ViewMatrix[0][0]);
		glUniformMatrix4fv(DepthBiasID, 1, GL_FALSE, &depthBiasMVP[0][0]);
        
		glUniform3f(lightInvDirID, lightInvDir.x, lightInvDir.y, lightInvDir.z);
        
		// Bind our texture in Texture Unit 0
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, Texture);
		// Set our "myTextureSampler" sampler to user Texture Unit 0
		glUniform1i(TextureID, 0);
        
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, depthTexture);
		glUniform1i(ShadowMapID, 1);
        
		// 1rst attribute buffer : vertices
		glEnableVertexAttribArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
		glVertexAttribPointer(
                              0,                  // attribute
                              3,                  // size
                              GL_FLOAT,           // type
                              GL_FALSE,           // normalized?
                              0,                  // stride
                              (void*)0            // array buffer offset
                              );
        
		// 2nd attribute buffer : UVs
		glEnableVertexAttribArray(1);
		glBindBuffer(GL_ARRAY_BUFFER, uvbuffer);
		glVertexAttribPointer(
                              1,                                // attribute
                              2,                                // size
                              GL_FLOAT,                         // type
                              GL_FALSE,                         // normalized?
                              0,                                // stride
                              (void*)0                          // array buffer offset
                              );
        
		// 3rd attribute buffer : normals
		glEnableVertexAttribArray(2);
		glBindBuffer(GL_ARRAY_BUFFER, normalbuffer);
		glVertexAttribPointer(
                              2,                                // attribute
                              3,                                // size
                              GL_FLOAT,                         // type
                              GL_FALSE,                         // normalized?
                              0,                                // stride
                              (void*)0                          // array buffer offset
                              );
        
		// Index buffer
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, elementbuffer);
        
		// Draw the triangles !
		glDrawElements(
                       GL_TRIANGLES,      // mode
                       indices.size(),    // count
                       GL_UNSIGNED_SHORT, // type
                       (void*)0           // element array buffer offset
                       );
        
		glDisableVertexAttribArray(0);
		glDisableVertexAttribArray(1);
		glDisableVertexAttribArray(2);
        
        
		// Optionally render the shadowmap (for debug only)
        
		// Render only on a corner of the window (or we we won't see the real rendering...)
		glViewport(0,0,512,512);
        
		// Use our shader
		glUseProgram(quad_programID);
        
		// Bind our texture in Texture Unit 0
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, depthTexture);
        glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_NONE );
//        glTexParameteri( GL_TEXTURE_2D, GL_DEPTH_TEXTURE_MODE, GL_LUMINANCE );
		// Set our "renderedTexture" sampler to user Texture Unit 0
		glUniform1i(texID, 0);
        
		// 1rst attribute buffer : vertices
		glEnableVertexAttribArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, quad_vertexbuffer);
		glVertexAttribPointer(
                              0,                  // attribute 0. No particular reason for 0, but must match the layout in the shader.
                              3,                  // size
                              GL_FLOAT,           // type
                              GL_FALSE,           // normalized?
                              0,                  // stride
                              (void*)0            // array buffer offset
                              );
        
		// Draw the triangle !
		// You have to disable GL_COMPARE_R_TO_TEXTURE above in order to see anything !
		glDrawArrays(GL_TRIANGLES, 0, 6); // 2*3 indices starting at 0 -> 2 triangles
		glDisableVertexAttribArray(0);
        
        
        // Swap buffers
		glfwSwapBuffers();
		glfwPollEvents();
		running = glfwGetWindowParam(GLFW_OPENED);
	}
    
	// Cleanup VBO and shader
	glDeleteBuffers(1, &vertexbuffer);
	glDeleteBuffers(1, &uvbuffer);
	glDeleteBuffers(1, &normalbuffer);
	glDeleteBuffers(1, &elementbuffer);
	glDeleteProgram(programID);
	glDeleteProgram(depthProgramID);
	glDeleteProgram(quad_programID);
	glDeleteTextures(1, &Texture);
    
	glDeleteFramebuffers(1, &FramebufferName);
	glDeleteTextures(1, &depthTexture);
	glDeleteBuffers(1, &quad_vertexbuffer);
	glDeleteVertexArrays(1, &VertexArrayID);
    
	// Close OpenGL window and terminate GLFW
	glfwTerminate();
    
	return 0;
}

