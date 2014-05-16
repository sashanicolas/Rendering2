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
//#include <glm/gtx/transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/rotate_vector.hpp>

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
    void Load ();
    void Unload ();
    void SetUniform (const char* name, float x);
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
    void SetUniformMVP(){
        // Projection matrix : 45° Field of View, 4:3 ratio, display range : 0.1 unit <-> 100 units
        Projection = glm::perspective(45.0f, 1.0f/*4.0f / 3.0f*/, 0.1f, 100.0f);
        
        // Camera matrix
        View = glm::lookAt(
                           glm::vec3(c.x,c.y,c.z), // Camera is at (x,y,z), in World Space, bom = 0,-2.0f,4.0f
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
        
        lightPos = glm::vec3(0,7,7);//bom = 5.0f, 4.0f, 4.0f
        LightID = glGetUniformLocation(shaderProgram, "LightPosition_worldspace" );
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
    GLfloat vertices_position[500000];
    GLfloat colors[500000];
    GLfloat normals[500000];
    glm::mat4 Model;
    
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
    
    std::vector<GLfloat> Vertices;
    std::vector<GLfloat> Normals;
    std::vector<GLfloat> Colors;
    std::vector<GLushort> Indices;
    unsigned int rings;
    unsigned int sectors;
    
    
    GLfloat vertices_position[500000];
    GLfloat colors[500000];
    GLfloat normals[500000];
    glm::mat4 Model;
    float x, y, z;
    
    //buffers
    GLuint vertexbuffer, colorsbuffer, normalsbuffer;
    
    Sphere (int slices, int stack){
        this->m_nx = slices;
        this->m_ny = stack;
        y=0.2f;
        
        rings = stack;
        sectors = slices;
        
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
        /*float const R = 1./(float)(rings-1);
         float const S = 1./(float)(sectors-1);
         int r, s;
         
         Vertices.resize(rings * sectors * 3);
         Normals.resize(rings * sectors * 3);
         Colors.resize(rings * sectors * 2);
         std::vector<GLfloat>::iterator v = Vertices.begin();
         std::vector<GLfloat>::iterator n = Normals.begin();
         std::vector<GLfloat>::iterator t = Colors.begin();
         
         float radius = 1.0;
         for(r = 0; r < rings; r++) for(s = 0; s < sectors; s++) {
         float const y = sin( -M_PI_2 + M_PI * r * R );
         float const x = cos(2*M_PI * s * S) * sin( M_PI * r * R );
         float const z = sin(2*M_PI * s * S) * sin( M_PI * r * R );
         
         *t++ = s*S;
         *t++ = r*R;
         
         *v++ = x * radius;
         *v++ = y * radius;
         *v++ = z * radius;
         
         *n++ = x;
         *n++ = y;
         *n++ = z;
         }
         
         Indices.resize(rings * sectors * 4);
         std::vector<GLushort>::iterator i = Indices.begin();
         for(r = 0; r < rings-1; r++) for(s = 0; s < sectors-1; s++) {
         *i++ = r * sectors + s;
         *i++ = r * sectors + (s+1);
         *i++ = (r+1) * sectors + (s+1);
         *i++ = (r+1) * sectors + s;
         }*/
        
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
        
        /*
         k=0;
         for (int j=0; j<m_ny; j++) {
         float cor = (float)(j+1)/(float)m_ny;
         printf("cor vermelho: %f\n",cor);
         for (int i=0; i<m_nx; i++) {
         float cor2 = (float)(i+1)/(float)m_nx;
         for (int l=0; l<6; l++) {
         colors[k++]=cor2;
         colors[k++]=0;
         colors[k++]=0;
         
         }
         }
         }*/
        
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
         if(i==3 && j==3) mostra=1;
         if(mostra==1){
         printf("i=%d j=%d k=%d\n",i,j,k);
         
         printf("triangulo 1\n");
         printf("(%.4f,%.4f,%.4f)\n",vertices_position[18*k],vertices_position[18*k+1],vertices_position[18*k+2]);
         printf("(%.4f,%.4f,%.4f)\n",normals[18*k],normals[18*k+1],normals[18*k+2]);
         printf("(%.4f,%.4f,%.4f)\n",vertices_position[18*k+3],vertices_position[18*k+4],vertices_position[18*k+5]);
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

class DeferRender{
public:
    Shader * quadShader;
    Shader * lightingShader;
    
    GLuint texID;
    GLuint quad_vertexbuffer;
    
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
		glUniform1i(this->texID, 0);
        
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
    
    void render(){
        
    }
};

class FBO{
public:
    GLuint m_fbo;
    GLuint m_textures[3];
    GLuint m_depthTexture;
    
    GLuint FramebufferName;
//    GLuint renderedTexture;
    GLuint depthrenderbuffer;
    
    FBO(){
        m_fbo = 0;
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
Luz * luzes;

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
void DSPointLightsPass()
{
    //m_DSPointLightPassTech.Enable();
    //m_DSPointLightPassTech.SetEyeWorldPos(m_pGameCamera->GetPos());
    
    //Pipeline p;
    //p.SetCamera(m_pGameCamera->GetPos(), m_pGameCamera->GetTarget(), m_pGameCamera->GetUp());
    //p.SetPerspectiveProj(m_persProjInfo);
    
    //for (unsigned int i = 0 ; i < ARRAY_SIZE_IN_ELEMENTS(m_pointLight); i++) {
        //m_DSPointLightPassTech.SetPointLight(m_pointLight[i]);
        //p.WorldPos(m_pointLight[i].Position);
        //float BSphereScale = CalcPointLightBSphere(m_pointLight[i]);
        //p.Scale(BSphereScale, BSphereScale, BSphereScale);
        //m_DSPointLightPassTech.SetWVP(p.GetWVPTrans());
        //m_bsphere.Render();
    //}
}
void DSDirectionalLightPass()
{
    //m_DSDirLightPassTech.Enable();
    //m_DSDirLightPassTech.SetEyeWorldPos(m_pGameCamera->GetPos());
    //Matrix4f WVP;
    //WVP.InitIdentity();
    //m_DSDirLightPassTech.SetWVP(WVP);
    //m_quad.Render();
}
void BeginLightPasses()
{
    glEnable(GL_BLEND);
    glBlendEquation(GL_FUNC_ADD);
    glBlendFunc(GL_ONE, GL_ONE);
    
    m_gbuffer.BindForReading();
    glClear(GL_COLOR_BUFFER_BIT);
}

// ========== Main ==========
int main () {
    configuraContexto();
    
    // Create a vertex array object
    GLuint VertexArrayID;
    // Use a Vertex Array Object
    glGenVertexArrays(1, &VertexArrayID);
    glBindVertexArray(VertexArrayID);
    
    configuraCena();

    FBO * fbo = new FBO();
    fbo->Init(WINDOW_WIDTH, WINDOW_HEIGHT);
    DeferRender * dr = new DeferRender();
    
	// Create a rendering loop
	int running = GL_TRUE;
    
	while(running) {
        
        dr->setRenderToFrameBuffer(fbo->FramebufferName);
        
        displayCena();
        updateLuz();
        
		dr->showTexture(fbo->m_textures[2]);
		//dr->render();

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

void configuraCena(){
    c.x =0;
    c.y =4;
    c.z =8;
    
    myShader = new Shader("shaders6/vert.cpp","shaders6/frag.cpp");

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
