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

struct Camera{
    float x, y, z;
};
Camera c;

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
        // create a program
        this->VertexShader("StandardShading.vertexshader");
        this->FragmentShader("StandardShading.fragmentshader");
        
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
        m = glGetUniformLocation(shaderProgram, name );
        glUniformMatrix4fv(m, 1, GL_FALSE, glm::value_ptr(Model));
        
    }
    
    void SetUniformMVP(){
        // Projection matrix : 45° Field of View, 4:3 ratio, display range : 0.1 unit <-> 100 units
        Projection = glm::perspective(45.0f, 1.0f/*4.0f / 3.0f*/, 0.1f, 100.0f);
        
        // Camera matrix
        View = glm::lookAt(
                           glm::vec3(0,20,-5), // Camera is at (x,y,z), in World Space, bom = 0,-2.0f,4.0f
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
        
        lightPos = glm::vec3(c.x,c.y,c.z);//bom = 5.0f, 4.0f, 4.0f
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
//            std::cerr << "Shader compilation failed with this message:" << std::endl;
//            std::vector<char> compilation_log(512);
//            glGetShaderInfoLog(shader, compilation_log.size(), NULL, &compilation_log[0]);
//            std::cerr << &compilation_log[0] << std::endl;
            glfwTerminate();
            exit(-1);
        }
        return shader;
    }
    
    
};


int main( void )
{
	// Initialise GLFW
	if( !glfwInit() )
	{
		fprintf( stderr, "Failed to initialize GLFW\n" );
		return -1;
	}
    
    // Open a window and attach an OpenGL rendering context to the window surface
	if( !glfwOpenWindow(800, 800, 8, 8, 8, 0, 0, 0, GLFW_WINDOW)) {
        fprintf( stderr, "Failed to open GLFW window. If you have an Intel GPU, they are not 3.3 compatible. Try the 2.1 version of the tutorials.\n" );
		glfwTerminate();
		exit(-1);
	}
    
	
	// Initialize GLEW
	glewExperimental = true; // Needed for core profile
	if (glewInit() != GLEW_OK) {
		fprintf(stderr, "Failed to initialize GLEW\n");
		return -1;
	}
    
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
//	GLuint programID = LoadShaders( "StandardShading.vertexshader", "StandardShading.fragmentshader" );
    
    Shader * myShader;
    myShader = new Shader();
    
	// Get a handle for our "LightPosition" uniform
	glUseProgram(myShader->programID());
	GLuint LightID = glGetUniformLocation(myShader->programID(), "LightPosition_worldspace");
    
    // Create a rendering loop
	int running = GL_TRUE;
    
	do{
        
		// Clear the screen
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        
		// Use our shader
		glUseProgram(myShader->programID());
        
		
        
        
		// Swap buffers
        glfwSwapBuffers();
        
		// Pool for events
		glfwPollEvents();
		// Check if the window was closed
		running = glfwGetWindowParam(GLFW_OPENED);
        
        
	} // Check if the ESC key was pressed or the window was closed
//	while( glfwGetKey(window, GLFW_KEY_ESCAPE ) != GLFW_PRESS &&
//          glfwWindowShouldClose(window) == 0 );
    while (running);
    
	// Cleanup VBO and shader
	glDeleteBuffers(1, &vertexbuffer);
	glDeleteBuffers(1, &uvbuffer);
	glDeleteBuffers(1, &normalbuffer);
	glDeleteProgram(myShader->programID());
	glDeleteVertexArrays(1, &VertexArrayID);
    
	// Close OpenGL window and terminate GLFW
	glfwTerminate();
    
	return 0;
}

