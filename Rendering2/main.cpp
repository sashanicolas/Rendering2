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

typedef struct MyVertex
{
    float x, y, z;        //Vertex
    float nx, ny, nz;     //Normal
    float r, g, b, a;     //color
}MyVertex;

class Shader{
public:
    //================ Variaveis ================//
    // Create a vertex array object
    GLuint vao;
    // Create a Vector Buffer Object that will store the vertices on video memory
    GLuint vbo;
    // id para o programa e os shaders
    GLuint shaderProgram, vertexShader, fragmentShader;
    //model view projection matrix
    //glm::mat4 MVP;
    
    //================ Metodos ================//
    Shader (GLfloat *vertices_position, GLfloat *colors, GLfloat *normals,
            std::size_t v, std::size_t c, std::size_t n){
        
        // Use a Vertex Array Object
        glGenVertexArrays(1, &vao);
        glBindVertexArray(vao);
        
        //generate buffer object names
        glGenBuffers(1, &vbo);
        
        // Allocate space for vertex positions and colors
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferData(GL_ARRAY_BUFFER, v+c+n, NULL, GL_STATIC_DRAW);
        
        // Transfer the vertex positions:
        glBufferSubData(GL_ARRAY_BUFFER, 0, v, vertices_position);
        // Transfer the vertex colors:
        glBufferSubData(GL_ARRAY_BUFFER, v, c, colors);
        // Transfer the vertex normals:
        glBufferSubData(GL_ARRAY_BUFFER, v+c, n, normals);
        
        // create a program
        this->VertexShader("shaders/vert.cpp");
        this->FragmentShader("shaders/frag.cpp");
        
        shaderProgram = glCreateProgram();
        glAttachShader(shaderProgram, vertexShader);
        glAttachShader(shaderProgram, fragmentShader);
        
        // Flag the shaders for deletion
        glDeleteShader(vertexShader);
        glDeleteShader(fragmentShader);
        
        this->Link();
        
        // Get the location of the attributes that enters in the vertex shader
        GLint position_attribute = glGetAttribLocation(shaderProgram, "position");
        // Specify how the data for position can be accessed
        glVertexAttribPointer(position_attribute, 3, GL_FLOAT, GL_FALSE, 0, 0);
        // Enable the attribute
        glEnableVertexAttribArray(position_attribute);
        
        // Color attribute
        GLint color_attribute = glGetAttribLocation(shaderProgram, "color");
        glVertexAttribPointer(color_attribute, 3, GL_FLOAT, GL_FALSE, 0, (GLvoid *)v);
        glEnableVertexAttribArray(color_attribute);
        
        // Normal attribute
        GLint normal_attribute = glGetAttribLocation(shaderProgram, "normal");
        glVertexAttribPointer(normal_attribute, 3, GL_FLOAT, GL_FALSE, 0, (GLvoid *)(v+c));
        glEnableVertexAttribArray(normal_attribute);
        
        SetUniformMVP();
    }
    ~Shader ();
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
    
    void SetUniformMVP(){
        //glm::mat4 MVP;
        //MVP = glm::translate(MVP, glm::vec3(0.5f, 0.5f, 0.0f));
        
        // Projection matrix : 45° Field of View, 4:3 ratio, display range : 0.1 unit <-> 100 units
        glm::mat4 Projection = glm::perspective(45.0f, 4.0f / 3.0f, 0.1f, 100.0f);
        // Camera matrix
        glm::mat4 View = glm::lookAt(
                                     glm::vec3(0,2,4), // Camera is at (4,3,3), in World Space
                                     glm::vec3(0,0,0), // and looks at the origin
                                     glm::vec3(0,1,0) // Head is up (set to 0,-1,0 to look upside-down)
                                     );
        // Model matrix : an identity matrix (model will be at the origin)
        glm::mat4 Model = glm::mat4(1.0f); // Changes for each model !
        // Our ModelViewProjection : multiplication of our 3 matrices
        glm::mat4 MVP = Projection * View * Model; // Remember, matrix multiplication is the other way around
        
        // Transfer the transformation matrices to the shader program
        GLint mvp = glGetUniformLocation(shaderProgram, "MVP" );
        glUniformMatrix4fv(mvp, 1, GL_FALSE, glm::value_ptr(MVP));
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


const double PI = 3.1415926535897;
int m_nx = 10, m_ny = 10;
int Index (int i, int j) {
    return i + j*(m_nx);
}

class Grid {
    // Create a grid with nx x ny quadrilateral.
    // The grid coordinates varies from 0.0 to 1.0 in the xz plane
public:
    int m_nx, m_ny;
    GLfloat vertices_position[50000];
    GLfloat colors[50000];
    GLfloat normals[50000];
    
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
                
                vertices_position[18*k+9] = (float)i / (float)m_nx;
                vertices_position[18*k+10] = 0.0f;
                vertices_position[18*k+11] = (float)j / (float)m_ny;
                
                vertices_position[18*k+12] = ((float)i+1) / (float)m_nx;
                vertices_position[18*k+13] = 0.0f;
                vertices_position[18*k+14] = ((float)j+1) / (float)m_ny;
                
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
        
        //glm::mat4 myScalingMatrix = glm::scale(2.0f, 2.0f ,2.0f);
        
    }
    int getSizeV(){
        return m_nx*m_ny*6*3; //10 vezes 10 sub quadrados com dois triangulos. Cada triangulo 3 vertices. Cada Vertice sao 3 floats
    }
};

// Called when the window is resized
void GLFWCALL window_resized(int width, int height);

// Called for keyboard events
void keyboard(int key, int action);

// Render scene
void display(GLuint &vao);

void init();

// Create a vertex array object
GLuint vao;

Shader *gridShader;

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
	if( !glfwOpenWindow(800, 800, 8, 8, 8, 0, 0, 0, GLFW_WINDOW)) {
		std::cerr << "Failed to open a window! I'm out!" << std::endl;
		glfwTerminate();
		exit(-1);
	}
    
	// Register a callback function for window resize events
	glfwSetWindowSizeCallback( window_resized );
    
	// Register a callback function for keyboard pressed events
	glfwSetKeyCallback(keyboard);
    
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
    
	
    init();
    
	// Create a rendering loop
	int running = GL_TRUE;
    
	while(running) {
		// Display scene
        display(gridShader->vao);
        
        // Swap front and back buffers
        glfwSwapBuffers();
        
		// Pool for events
		glfwPollEvents();
		// Check if the window was closed
		running = glfwGetWindowParam(GLFW_OPENED);
	}
    
	// Terminate GLFW
	glfwTerminate();
    
	return 0;
}

void init(){
    // 4 triangles to be rendered
	GLfloat vertices_position[] = {
		0.0, 0.0, 0.0,
		0.5, 0.0, 0.0,
		0.5, 0.5, 0.0,
        
		0.0, 0.0, 0.0,
		0.0, 0.5, 0.0,
		-0.5, 0.5, 0.0,
        
		0.0, 0.0, 0.0,
		-0.5, 0.0, 0.0,
		-0.5, -0.5, 0.0,
        
		0.0, 0.0, 0.0,
		0.0, -0.5, 0.0,
		0.5, -0.5, 0.0,
	};
    
	GLfloat colors[36];
    GLfloat normals[36];
    
    // Initialize the random seed from the system time
	srand(time(NULL));
    
	// Fill colors with random numbers from 0 to 1, use continuous polynomials for r,g,b:
	int k = 0;
	for(int i = 0; i < sizeof(colors)/sizeof(float)/3; ++i) {
		float t = (float)rand()/(float)RAND_MAX;
		colors[k] = 9*(1-t)*t*t*t;
		k++;
		colors[k] = 15*(1-t)*(1-t)*t*t;
		k++;
		colors[k] = 8.5*(1-t)*(1-t)*(1-t)*t;
		k++;
        
	}
    // Fill normals, 0,0,1
    k=0;
	for(int i = 0; i < sizeof(colors)/sizeof(float)/3; ++i) {
        normals[k] = 0;
		k++;
		normals[k] = 0;
		k++;
		normals[k] = 1.0f;
		k++;
        
	}
    
    Grid *g = new Grid(10,10);
    g->genGrid();
    gridShader = new Shader(g->vertices_position, g->colors, g->normals,
                          g->getSizeV(),g->getSizeV(),g->getSizeV());
    
    //gridShader = new Shader(vertices_position, colors, normals,
      //                      sizeof(vertices_position),sizeof(colors),sizeof(normals));
    
    glm::mat4 Model, View, Projection;
    // Set the projection matrix
    Projection = glm::ortho(-4.0f/3.0f, 4.0f/3.0f, -1.0f, 1.0f, -1.0f, 1.0f);
    // Translation
    Model = glm::translate(Model, glm::vec3(0.1f, 0.2f, 0.5f));
    // Rotation around Oz with 45 degrees
    Model = glm::rotate(Model, 45.0f, glm::vec3(0.0f, 0.0f, 1.0f));
    
    // Transfer the transformation matrices to the shader program
    //  GLint model = glGetUniformLocation(shaderProgram, "Model" );
    //    glUniformMatrix4fv(model, 1, GL_FALSE, glm::value_ptr(Model));
    
}

// Render scene
void display(GLuint &vao) {
	glClear(GL_COLOR_BUFFER_BIT);
    
	glBindVertexArray(vao);
	glDrawArrays(GL_TRIANGLES, 0, 12);
    
	// Swap front and back buffers
    //glfwSwapBuffers();
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
	if(key == 'Q' && action == GLFW_PRESS) {
		glfwTerminate();
		exit(0);
	}
}
