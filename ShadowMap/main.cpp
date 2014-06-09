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
#define WINDOW_WIDTH  1024
#define WINDOW_HEIGHT 1024

glm::vec3 camera = glm::vec3(-5,5,0);
glm::vec3 centro = glm::vec3(0,0,0);
glm::vec3 lightInvDir = glm::vec3(-5,5,5);

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
        Projection = glm::perspective(45.0f, 1.0f, 0.1f, 100.0f);
        
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
        lightPos = glm::vec3(4,5,5);//bom = 5.0f, 4.0f, 4.0f
        LightID = glGetUniformLocation(shaderProgram, "LightPosition_worldspace" );
		glUniform3f(LightID, lightPos.x, lightPos.y, lightPos.z);
    }
    
    void SetUniformMVP2(){
        // Projection matrix : 45° Field of View, 4:3 ratio, display range : 0.1 unit <-> 100 units
        Projection = glm::perspective(45.0f, 1.0f, 0.1f, 100.0f);

        // Camera matrix
        View = glm::lookAt(camera,centro,glm::vec3(0,1,0));
        
        // Our ModelViewProjection : multiplication of our 3 matrices
        MVP = Projection * View * Model; // Remember, matrix multiplication is the other way around
        // Transposta da inversa
        ITMV = glm::transpose(glm::inverse(View * Model));
        
        // Transfer the transformation matrices to the shader program
        m = glGetUniformLocation(shaderProgram, "M" );
        glUniformMatrix4fv(m, 1, GL_FALSE, glm::value_ptr(Model));
        
        // Transfer the transformation matrices to the shader program
        v = glGetUniformLocation(shaderProgram, "V" );
        glUniformMatrix4fv(v, 1, GL_FALSE, glm::value_ptr(View));
        
        // Transfer the transformation matrices to the shader program
        mvp = glGetUniformLocation(shaderProgram, "MVP" );
        glUniformMatrix4fv(mvp, 1, GL_FALSE, glm::value_ptr(MVP));
        
        // Transfer the transformation matrices to the shader program
        itmv = glGetUniformLocation(shaderProgram, "NormalMatrix" );
        glUniformMatrix4fv(itmv, 1, GL_FALSE, glm::value_ptr(ITMV));

        LightID = glGetUniformLocation(shaderProgram, "LightPosition_worldspace" );
		glUniform3f(LightID, lightInvDir.x, lightInvDir.y, lightInvDir.z);
        
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
        
        glm::mat4 biasMatrix(
                             0.5, 0.0, 0.0, 0.0,
                             0.0, 0.5, 0.0, 0.0,
                             0.0, 0.0, 0.5, 0.0,
                             0.5, 0.5, 0.5, 1.0
                             );
        glm::mat4 depthBiasMVP = biasMatrix * depthMVP;
        
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
//        Model = glm::rotate(Model, 150.0f,glm::vec3(0,1,0));

        myShader->setModelMatrix(Model);
        
        glm::mat4 biasMatrix(
                             0.5, 0.0, 0.0, 0.0,
                             0.0, 0.5, 0.0, 0.0,
                             0.0, 0.0, 0.5, 0.0,
                             0.5, 0.5, 0.5, 1.0
                             );
        glm::mat4 depthBiasMVP = biasMatrix * depthMVP;

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

Luz * luzes[50];

class ShadowMap{
public:
    GLuint FramebufferName;
    GLuint depthTexture;
    
    ShadowMap(){
        FramebufferName = 0;
        depthTexture = 0;
    }
    
    bool Init(unsigned int WindowWidth, unsigned int WindowHeight){
        // The framebuffer, which regroups 0, 1, or more textures, and 0 or 1 depth buffer.
        glGenFramebuffers(1, &FramebufferName);
        glBindFramebuffer(GL_FRAMEBUFFER, FramebufferName);
        
        // Depth texture. Slower than a depth buffer, but you can sample it later in your shader
        glGenTextures(1, &depthTexture);
        glBindTexture(GL_TEXTURE_2D, depthTexture);
        glTexImage2D(GL_TEXTURE_2D, 0,GL_DEPTH_COMPONENT32, WindowWidth, WindowHeight, 0,GL_DEPTH_COMPONENT, GL_FLOAT, 0);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE);
        
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
Sphere * s[10][10];
Grid *g;
Sphere * luz;


// ========== Main ==========
int main () {

    configuraContexto();

    // Create a vertex array object
    GLuint VertexArrayID;
    // Use a Vertex Array Object
    glGenVertexArrays(1, &VertexArrayID);
    glBindVertexArray(VertexArrayID);
    
    configuraCena();
    // Dark blue background
	glClearColor(0,0,0,0);
	// Enable depth test
	glEnable(GL_DEPTH_TEST);
	// Accept fragment if it closer to the camera than the former one
	glDepthFunc(GL_LESS);
	// Cull triangles which normal is not towards the camera
	glEnable(GL_CULL_FACE);
    
    //============================================================
    
	ShadowMap * shadowMap = new ShadowMap();
    shadowMap->Init(1024, 1024);
    
    //============================================================
    Shader * depthShader = new Shader("shadowmapShaders/DepthRTT.vertexshader",
                                      "shadowmapShaders/DepthRTT.fragmentshader");
    
    // Compute the MVP matrix from the light's point of view
    glm::mat4 depthProjectionMatrix = glm::ortho<float>(-5,5,-5,5,-20,20);
    glm::mat4 depthViewMatrix = glm::lookAt(lightInvDir, glm::vec3(0,0,0), glm::vec3(0,1,0));
    depthShader->setProjectionMatrix(depthProjectionMatrix);
    depthShader->setViewMatrix(depthViewMatrix);
    
    //============================================================
    Shader * shadowShader = new Shader("shadowmapShaders/ShadowMapping_SimpleVersion.vertexshader",
                                       "shadowmapShaders/ShadowMapping_SimpleVersion.fragmentshader");
//    Shader * shadowShader = new Shader("shadowmapShaders/ShadowMapping.vertexshader",
//                                       "shadowmapShaders/ShadowMapping.fragmentshader");
    camera = glm::vec3(0,4,8);

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
    
    glDisable(GL_CULL_FACE);
    //============================================================
    
    // Create a rendering loop
	int running = GL_TRUE;
	while(running) {
//        glBindTexture(GL_TEXTURE_2D, shadowMap->depthTexture);
//        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE);
        //============================================================
        
//        lightInvDir = glm::rotateY(lightInvDir, 0.3f);
//        depthViewMatrix = glm::lookAt(lightInvDir, glm::vec3(0,0,0), glm::vec3(0,1,0));
//        depthShader->setViewMatrix(depthViewMatrix);
        
        // Render to our framebuffer
        glBindFramebuffer(GL_FRAMEBUFFER, shadowMap->FramebufferName);
        glViewport(0,0,1024,1024); // Render on the whole framebuffer, complete from the lower left corner to the upper right
        
        // We don't use bias in the shader, but instead we draw back faces,
        // which are already separated from the front faces by a small distance
        // (if your geometry is made this way)
//        glEnable(GL_CULL_FACE);
        //glCullFace(GL_FRONT); // Cull front-facing triangles -> draw only back-facing triangles
//        glCullFace(GL_BACK); // Cull back-facing triangles -> draw only front-facing triangles
        
        // Clear
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
        
//        glEnable(GL_CULL_FACE);
//        glCullFace(GL_BACK); // Cull back-facing triangles -> draw only front-facing triangles
        
        // Clear the screen
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        
//        camera = glm::rotateY(camera, 0.3f);

        g->drawShadowShader(shadowShader,shadowMap->depthTexture);
        for (int i=0;i<10;i++){
            for (int j=0; j<10; j++) {
                s[i][j]->drawShadowShader(shadowShader,shadowMap->depthTexture);
            }
        }

        //*/
        
        //============================================================
        
        // Render Shadow Map
        // Render only on a corner of the window (or we we won't see the real rendering...)
        /*glViewport(0,0,128,128);

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
        glDisableVertexAttribArray(0); //*/
        //============================================================
        
        //Swap buffers
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

glm::vec3 randPos(){
    return glm::vec3(rand()%20 -10, rand()%20-10, rand()%20 -10);
}
glm::vec3 randCor(){
    return glm::vec3((rand()%10+1)/10.0, (rand()%10+1)/10.0, (rand()%10+1)/10.0);
}

void configuraCena(){
    //inicializa os objetos
    for (int i=0;i<10;i++){
        for (int j=0; j<10; j++) {
            s[i][j] = new Sphere(64,64);
            s[i][j]->setPositionXZ(i,j);
        }
    }
    g = new Grid(64,64);
    luz = new Sphere(32,32);
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
    glfwSetWindowTitle( "Sasha Nicolas - Shadow Mapping with PCF");
    
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
//            s[i][j]->draw(myShader);
        }
    }
//    g->draw(myShader);
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
	if( key == GLFW_KEY_ESC && action == GLFW_PRESS) {
		glfwTerminate();
		exit(0);
	}
    float offset = 0.5;
    if( key == 'W' && action == GLFW_PRESS) {
        //printf("frente\n");
        glm::vec3 dir = centro - camera;
        dir = glm::normalize(dir);
        centro = centro + dir;
        camera = camera + dir;
	}
    if( key == 'S' && action == GLFW_PRESS) {
        //printf("tras\n");
        glm::vec3 dir = centro - camera;
        dir = glm::normalize(dir);
        centro = centro - dir;
        camera = camera - dir;
	}
    if( key == 'D' && action == GLFW_PRESS) {
        //printf("direita\n");
        glm::vec3 dir = centro - camera;
        dir = glm::normalize(dir);
        dir = glm::cross(dir, glm::vec3 (0,1,0));
        centro = centro + dir*offset;
        camera = camera + dir*offset;
	}
    if( key == 'A' && action == GLFW_PRESS) {
        //printf("esquerda\n");
        glm::vec3 dir = centro - camera;
        dir = glm::normalize(dir);
        dir = glm::cross(dir, glm::vec3 (0,1,0));
        centro = centro - dir*offset;
        camera = camera - dir*offset;
	}
    if( key == GLFW_KEY_UP && action == GLFW_PRESS) {
        glm::vec3 dir = centro - camera;
        dir = glm::rotateX(dir, 1.0f);
        centro = camera + dir;
	}
    if( key == GLFW_KEY_DOWN && action == GLFW_PRESS) {
        glm::vec3 dir = centro - camera;
        dir = glm::rotateX(dir, -1.0f);
        centro = camera + dir;
	}
    if( key == GLFW_KEY_RIGHT && action == GLFW_PRESS) {
        glm::vec3 dir = centro - camera;
        dir = glm::rotateY(dir, -1.0f);
        centro = camera + dir;
	}
    if( key == GLFW_KEY_LEFT && action == GLFW_PRESS) {
        glm::vec3 dir = centro - camera;
        dir = glm::rotateY(dir, 1.0f);
        centro = camera + dir;
	}
}
