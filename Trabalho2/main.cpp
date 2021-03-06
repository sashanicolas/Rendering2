// Draw four triangles on a red background
#include <GL/glew.h>
#include <GL/glfw.h>
#include <cstdlib>
#include <iostream>
#include <fstream>
#include <vector>
#include <ctime>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "Camera.hpp"       // Include our Camera header so we can work with Camera objects

// Define a pointer to our camera object
Camera *cam;

class Shader{
public:
    //================ Variaveis ================//
    // id para o programa e os shaders
    GLuint shaderProgram, vertexShader, fragmentShader;
    glm::mat4 Model;
    
    //================ Metodos ================//
    Shader (){
        // create a program
        this->VertexShader("shaders1/vert.cpp");
        this->FragmentShader("shaders1/frag.cpp");
        
        shaderProgram = glCreateProgram();
        glAttachShader(shaderProgram, vertexShader);
        glAttachShader(shaderProgram, fragmentShader);
        
        // Flag the shaders for deletion
        glDeleteShader(vertexShader);
        glDeleteShader(fragmentShader);
        
        this->Link();
        
        //SetUniformMVP();
    }
    ~Shader ();
    
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
    void Load ();
    void Unload ();
    void SetUniform (const char* name, float x);
    void SetUniform (const char* name, float x, float y);
    void SetUniform (const char* name, float x, float y, float z);
    void SetUniform (const char* name, float x, float y, float z, float w);
    void SetUniform (const char* name, int size, int count, float* v);
    void SetUniformI (const char* name, int x);
    void SetUniformI (const char* name, int x, int y);
    void SetUniformI (const char* name, int x, int y, int z);
    void SetUniformI (const char* name, int x, int y, int z, int w);
    void SetUniformI (const char* name, int size, int count, int* v) ;
    void SetUniformMatrix (const char* name, int row, int col, int count, float* v);
    
    void SetUniform (const char* name, glm::mat4 Model){
        this->Model = Model;
        // Transfer the transformation matrices to the shader program
        GLint m = glGetUniformLocation(shaderProgram, name );
        glUniformMatrix4fv(m, 1, GL_FALSE, glm::value_ptr(Model));
        
    }
    void setModelMatrix(glm::mat4 Model){
        this->Model = Model;
    }
    
    void SetUniformMVP(){
        // Projection matrix : 45° Field of View, 4:3 ratio, display range : 0.1 unit <-> 100 units
        glm::mat4 Projection = glm::perspective(45.0f, 1.0f/*4.0f / 3.0f*/, 0.1f, 100.0f);
        // Camera matrix
        glm::mat4 View = glm::lookAt(
                                     glm::vec3(cam->getXPos(), cam->getYPos(), cam->getZPos()), // Camera is at (x,y,z), in World Space
                                     glm::vec3(0,0,0), // and looks at the origin
                                     glm::vec3(0,1,0) // Head is up (set to 0,-1,0 to look upside-down)
                                     );
        // Model matrix : an identity matrix (model will be at the origin)
        //glm::mat4 Model = glm::mat4(1.0f); // Changes for each model !
        //Model = glm::translate(Model, glm::vec3(-4.0f, 0, -4.0f));
        //Model = glm::scale(Model, glm::vec3(8,1,8));

        // Our ModelViewProjection : multiplication of our 3 matrices
        glm::mat4 MVP = Projection * View * Model; // Remember, matrix multiplication is the other way around
        
        // Transfer the transformation matrices to the shader program
        GLint mvp = glGetUniformLocation(shaderProgram, "MVP" );
        glUniformMatrix4fv(mvp, 1, GL_FALSE, glm::value_ptr(MVP));
        
        glm::mat4 ITMV = glm::transpose(glm::inverse(View * Model));
        // Transfer the transformation matrices to the shader program
        GLuint itmv = glGetUniformLocation(shaderProgram, "NormalMatrix" );
        glUniformMatrix4fv(itmv, 1, GL_FALSE, glm::value_ptr(ITMV));
        
        // Transfer the transformation matrices to the shader program
        GLint v = glGetUniformLocation(shaderProgram, "V" );
        glUniformMatrix4fv(v, 1, GL_FALSE, glm::value_ptr(View));
        
        glm::vec3 lightPos = glm::vec3(0, 8, 6);
        GLint LightID = glGetUniformLocation(shaderProgram, "LightPosition_worldspace" );
		glUniform3f(LightID, lightPos.x, lightPos.y, lightPos.z);
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

class Grid {
    // Create a grid with nx x ny quadrilateral.
    // The grid coordinates varies from 0.0 to 1.0 in the xz plane
public:
    int m_nx, m_ny;
    GLfloat vertices_position[50000];
    GLfloat colors[50000];
    GLfloat normals[50000];
    glm::mat4 Model;
    
    Grid (int nx, int ny){
        this->m_nx = nx;
        this->m_ny = ny;
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
        
        // Create a Vector Buffer Object that will store the vertices on video memory
        GLuint vertexbuffer;
        glGenBuffers(1, &vertexbuffer);
        glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
        glBufferData(GL_ARRAY_BUFFER, this->sizeOfAllFloats(), this->vertices_position, GL_STATIC_DRAW);
        
        // Create a Vector Buffer Object that will store the colors on video memory
        GLuint colorsbuffer;
        glGenBuffers(1, &colorsbuffer);
        glBindBuffer(GL_ARRAY_BUFFER, colorsbuffer);
        glBufferData(GL_ARRAY_BUFFER, this->sizeOfAllFloats(), this->colors, GL_STATIC_DRAW);
        
        // Create a Vector Buffer Object that will store the colors on video memory
        GLuint normalsbuffer;
        glGenBuffers(1, &normalsbuffer);
        glBindBuffer(GL_ARRAY_BUFFER, normalsbuffer);
        glBufferData(GL_ARRAY_BUFFER, this->sizeOfAllFloats(), this->normals, GL_STATIC_DRAW);
        
        
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
    float x, z;
    
    Sphere (int slices, int stack){
        this->m_nx = slices;
        this->m_ny = stack;
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

        /*k=0;
        for (int j=0; j<m_ny; j++) {
            for (int i=0; i<m_nx; i++) {

                glm::vec3 normal;// = getNormalVertex(i, j);
                if(i==3 && j==3) mostra=1;
                normal = getNormalVertex(i, j);
                if(mostra==1) printf("normal nomalizada %f,%f,%f\n",normal.x,normal.y,normal.z);
                mostra=0;
                normals[k++]= normal.x;
                normals[k++]= normal.y;
                normals[k++]= normal.z;
                normal = getNormalVertex(i+1, j+1);
                normals[k++]= normal.x;
                normals[k++]= normal.y;
                normals[k++]= normal.z;
                normal = getNormalVertex(i+1, j);
                normals[k++]= normal.x;
                normals[k++]= normal.y;
                normals[k++]= normal.z;
                
                normal = getNormalVertex(i, j);
                normals[k++]= normal.x;
                normals[k++]= normal.y;
                normals[k++]= normal.z;
                normal = getNormalVertex(i+1, j+1);
                normals[k++]= normal.x;
                normals[k++]= normal.y;
                normals[k++]= normal.z;
                normal = getNormalVertex(i, j+1);
                normals[k++]= normal.x;
                normals[k++]= normal.y;
                normals[k++]= normal.z;
            }
        }*/
        
        /*for (int j=0; j<m_ny; j++) {
            for (int i=0; i<m_nx; i++) {
                int k = Index(i, j);
                //if(i==3 && j==3) mostra=1;
                if(mostra==1){
                    printf("i=%d j=%d k=%d\n",i,j,k);
                    
                    printf("triangulo 1\n");
                    printf("(%.2f,%.2f,%.2f)\n",vertices_position[18*k],vertices_position[18*k+1],vertices_position[18*k+2]);
                    printf("(%.4f,%.4f,%.4f)\n",normals[18*k],normals[18*k+1],normals[18*k+2]);
                    printf("(%.2f,%.2f,%.2f)\n",vertices_position[18*k+3],vertices_position[18*k+4],vertices_position[18*k+5]);
                    printf("(%.4f,%.4f,%.4f)\n",normals[18*k+3],normals[18*k+4],normals[18*k+5]);
                    printf("(%.2f,%.2f,%.2f)\n",vertices_position[18*k+6],vertices_position[18*k+7],vertices_position[18*k+8]);
                    printf("(%.4f,%.4f,%.4f)\n",normals[18*k+6],normals[18*k+7],normals[18*k+8]);
                    
                    printf("triangulo 2\n");
                    printf("(%.2f,%.2f,%.2f)\n",vertices_position[18*k+9],vertices_position[18*k+10],vertices_position[18*k+11]);
                    printf("(%.2f,%.2f,%.2f)\n",vertices_position[18*k+12],vertices_position[18*k+13],vertices_position[18*k+14]);
                    printf("(%.2f,%.2f,%.2f)\n\n",vertices_position[18*k+15],vertices_position[18*k+16],vertices_position[18*k+17]);
                }
                mostra=0;
            }
        }*/
    }//end construtor
    
    void draw(Shader *myShader){
        GLuint shaderProgram = myShader->programID();

        // Create a Vector Buffer Object that will store the vertices on video memory
        GLuint vertexbuffer;
        glGenBuffers(1, &vertexbuffer);
        glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
        glBufferData(GL_ARRAY_BUFFER, this->sizeOfAllFloats(), this->vertices_position, GL_STATIC_DRAW);
        
        // Create a Vector Buffer Object that will store the colors on video memory
        GLuint colorsbuffer;
        glGenBuffers(1, &colorsbuffer);
        glBindBuffer(GL_ARRAY_BUFFER, colorsbuffer);
        glBufferData(GL_ARRAY_BUFFER, this->sizeOfAllFloats(), this->colors, GL_STATIC_DRAW);
        
        // Create a Vector Buffer Object that will store the colors on video memory
        GLuint normalsbuffer;
        glGenBuffers(1, &normalsbuffer);
        glBindBuffer(GL_ARRAY_BUFFER, normalsbuffer);
        glBufferData(GL_ARRAY_BUFFER, this->sizeOfAllFloats(), this->normals, GL_STATIC_DRAW);
        
        
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
        Model = glm::translate(Model, glm::vec3(x, 0.2f, z));
        Model = glm::scale(Model, glm::vec3(0.2f,0.2f,0.2f));

        myShader->setModelMatrix(Model);
        myShader->SetUniform("M", Model);
    }
    
    int mostra=0;
    glm::vec3 getNormalVertex(int i, int j){
        glm::vec3 normal = glm::vec3(0,0,0);
//        triangle ( v1, v2, v3 )
//        edge1 = v2-v1
//        edge2 = v3-v1
//        triangle.normal = cross(edge1, edge2).normalize()
        
//        vertex v1, v2, v3, ....
//        triangle tr1, tr2, tr3 // all share vertex v1
//        v1.normal = normalize( tr1.normal + tr2.normal + tr3.normal )

        if(i>0 && j<m_ny-1){ //triangulo 1
            if(mostra==1) printf("triangulo 1\n");
            normal = normal + getNormalTriangle(i-1, j, 1);
        }
        if(i>0 && j>0){ //triangulo 2 e 3
            if(mostra==1) printf("triangulo 2 e 3\n");
            normal = normal + getNormalTriangle(i-1, j-1, 0) + getNormalTriangle(i-1, j-1, 1);
        }
        if(i<m_nx-1 && j>0){ //triangulo 4
            if(mostra==1) printf("triangulo 4\n");
            normal = normal + getNormalTriangle(i, j-1, 0);
        }
        if(i<m_nx-1 && j<m_ny-1){ //triangulo 5 e 6
            if(mostra==1) printf("triangulo 5 e 6\n");
            normal = normal + getNormalTriangle(i, j, 1) + getNormalTriangle(i, j, 0);
//            float x = normal.x;

            if(mostra==1) printf("normal somada %f,%f,%f\n",normal.x,normal.y,normal.z);
        }
        
        return glm::normalize(normal);
    }
    
    glm::vec3 getNormalTriangle(int i, int j, int lado){
        if (lado == 0) { //a
            if(mostra==1) {
                printf("lado a\n");
                printf("edge 1 = v2(%d,%d) - v1(%d,%d)\n",i+1,j+1,i,j);
                printf("edge 2 = v3(%d,%d) - v1(%d,%d)\n",i,j+1,i,j);
                printf("cross(edge1, edge2)\n\n");
            }
            return glm::normalize( glm::cross(getVertexCoord(i+1, j+1)-getVertexCoord(i, j), getVertexCoord(i, j+1)-getVertexCoord(i, j)) );
        }else{ //b
            if(mostra==1) {
                printf("lado b\n");
                printf("edge 1 = v2(%d,%d) - v1(%d,%d)\n",i+1,j,i,j);
                printf("edge 2 = v3(%d,%d) - v1(%d,%d)\n",i+1,j+1,i,j);
                printf("v1 (%f,%f,%f)\n",getVertexCoord(i, j).x,getVertexCoord(i, j).y,getVertexCoord(i, j).z);
                printf("v2 (%f,%f,%f)\n",getVertexCoord(i+1, j).x,getVertexCoord(i+1, j).y,getVertexCoord(i+1, j).z);
                printf("v3 (%f,%f,%f)\n",getVertexCoord(i+1, j+1).x,getVertexCoord(i+1, j+1).y,getVertexCoord(i+1, j+1).z);
                printf("cross(edge1, edge2)\n\n");
            }
            return glm::normalize( glm::cross(getVertexCoord(i+1, j)-getVertexCoord(i, j), getVertexCoord(i+1, j+1)-getVertexCoord(i, j)) );
        }
        
    }
    
    glm::vec3 getVertexCoord(int i, int j){
        int k = Index(i, j);
        //printf("vertice em k=%d\n",k);
        return glm::vec3(vertices_position[18*k],vertices_position[18*k+1],vertices_position[18*k+2]);
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

// Called when the window is resized
void GLFWCALL window_resized(int width, int height);

// Called for keyboard events
void keyboard(int key, int action);

void init();

Shader * myShader;
Sphere * s[10][10];
Grid *g;

// Callback function to handle keypresses
void handleKeypress(int theKey, int theAction)
{
    if( theKey == 'Q' && theAction == GLFW_PRESS) {
		glfwTerminate();
		exit(0);
	}
	// If a key is pressed, toggle the relevant key-press flag
	if (theAction == GLFW_PRESS)
	{
		switch (theKey)
		{
            case 'W':
                cam->holdingForward = true;
                break;
            case 'S':
                cam->holdingBackward = true;
                break;
            case 'A':
                cam->holdingLeftStrafe = true;
                break;
            case 'D':
                cam->holdingRightStrafe = true;
                break;
            default:
                // Do nothing...
                break;
		}
	}
	else // If a key is released, toggle the relevant key-release flag
	{
		switch (theKey)
		{
            case 'W':
                cam->holdingForward = false;
                break;
            case 'S':
                cam->holdingBackward = false;
                break;
            case 'A':
                cam->holdingLeftStrafe = false;
                break;
            case 'D':
                cam->holdingRightStrafe = false;
                break;
            default:
                // Do nothing...
                break;
		}
	}
}

// Callback function to handle mouse movements
void handleMouseMove(int mouseX, int mouseY)
{
	cam->handleMouseMove(mouseX, mouseY);
}


int main () {
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
	if( !glfwOpenWindow(800, 800, 8, 8, 8, 8, 24, 8, GLFW_WINDOW)) {
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
	glClearColor(0.0f, 0.0f, 0.4f, 0.0f);
    // Enable depth test
	glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
	// Register a callback function for window resize events
	glfwSetWindowSizeCallback( window_resized );
    // Register a callback function for keyboard pressed events
//	glfwSetKeyCallback(keyboard);
    
    // Instantiate our pointer to a Camera object providing it the size of the window
	cam = new Camera(800, 800);
	// Set the mouse cursor to the centre of our window
	glfwSetMousePos(400, 400);
	// Specify the function which should execute when a key is pressed or released
	glfwSetKeyCallback(handleKeypress);
	// Specify the function which should execute when the mouse is moved
	glfwSetMousePosCallback(handleMouseMove);
    
    // Create a vertex array object
    GLuint VertexArrayID;
    // Use a Vertex Array Object
    glGenVertexArrays(1, &VertexArrayID);
    glBindVertexArray(VertexArrayID);
    
    init();
    
	// Create a rendering loop
	int running = GL_TRUE;
    
	while(running) {
		// Display scene

        // Calculate our camera movement
		cam->move(10.0);
        
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        g->draw(myShader);
        myShader->SetUniformMVP();
        glDrawArrays(GL_TRIANGLES, 0, g->numberOfPoints());
        
//        s[0][0]->draw(myShader);
//        myShader->SetUniformMVP();
//        glDrawArrays(GL_TRIANGLES, 0, s[0][0]->numberOfPoints());

        for (int i=0;i<10;i++){
            for (int j=0; j<10; j++) {
                s[i][j]->draw(myShader);
                myShader->SetUniformMVP();
                glDrawArrays(GL_TRIANGLES, 0, s[i][j]->numberOfPoints());
                
            }
        }
        
        // Swap front and back buffers
        glfwSwapBuffers();
        
		// Pool for events
		glfwPollEvents();
		// Check if the window was closed
		running = !glfwGetKey(GLFW_KEY_ESC) && glfwGetWindowParam(GLFW_OPENED);
	}
    
	// Terminate GLFW
	glfwTerminate();
    
	return 0;
}

void init(){
    myShader = new Shader();
    
//    s[0][0] = new Sphere(32,32);
//    s[0][0]->genSphere();
    
    for (int i=0;i<10;i++){
        for (int j=0; j<10; j++) {
            s[i][j] = new Sphere(32,32);
            s[i][j]->setPositionXZ(i,j);
            s[i][j]->genSphere();
        }
    }
    
    g = new Grid(20,20);
    g->genGrid();
}

// Called when the window is resized
void GLFWCALL window_resized(int width, int height) {
	// Use red to clear the screen
	glClearColor(1, 1, 1, 1);
    
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
