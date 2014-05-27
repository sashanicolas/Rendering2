
#include <GLUT/GLUT.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

const double PI = 3.1415926535897;
int m_nx = 10, m_ny = 10;
int Index (int i, int j) {
    return i + j*(m_nx);
}
GLfloat m_coord[900000], m_colors[900000], n_normal[900000];

class Grid {
    // Create a grid with nx x ny quadrilateral.
    // The grid coordinates varies from 0.0 to 1.0 in the xy plane
public:
    int m_nx, m_ny;
    Grid (int nx, int ny){
        this->m_nx = nx;
        this->m_ny = ny;
    }
    
    // Destroy a grid
    ~Grid ();
    
    // Draw a grid
    void Draw (){
        for (int j=0; j<m_ny; j++) {
            for (int i=0; i<m_nx; i++) {
                int k = Index(i, j);
                m_coord[18*k] = (float)i / m_nx;
                m_coord[18*k+1] = 0.0f;
                m_coord[18*k+2] = (float)j / m_ny;
                
                m_coord[18*k+3] = ((float)i+1) / m_nx;
                m_coord[18*k+4] = 0.0f;
                m_coord[18*k+5] = ((float)j+1) / m_ny;
                
                m_coord[18*k+6] = ((float)i+1) / m_nx;
                m_coord[18*k+7] = 0.0f;
                m_coord[18*k+8] = (float)j / m_ny;
                
                m_coord[18*k+9] = (float)i / m_nx;
                m_coord[18*k+10] = 0.0f;
                m_coord[18*k+11] = (float)j / m_ny;
                
                m_coord[18*k+12] = ((float)i+1) / m_nx;
                m_coord[18*k+13] = 0.0f;
                m_coord[18*k+14] = ((float)j+1) / m_ny;
                
                m_coord[18*k+15] = (float)i / m_nx;
                m_coord[18*k+16] = 0.0f;
                m_coord[18*k+17] = ((float)j+1) / m_ny;
            }
        }
        int k = 0;
        
        k = 0;
        for (int j=0; j<m_nx*m_ny*6; j++) {
            m_colors[k++]=0.4f;
            m_colors[k++]=0.6f;
            m_colors[k++]=0;
        }
        k = 0;
        
        for (int j=0; j<m_nx*m_ny*6; j++) {
            n_normal[k++]=0;
            n_normal[k++]=1;
            n_normal[k++]=0;
        }
        for (int j=0; j<m_nx*m_ny*6; j++) {
            //       printf("%d = %f %f %f\n",j,m_coord[j*3],m_coord[j*3+1],m_coord[j*3+2]);
        }
        // enble and specify pointers to vertex arrays
        glEnableClientState(GL_NORMAL_ARRAY);
        glEnableClientState(GL_COLOR_ARRAY);
        glEnableClientState(GL_VERTEX_ARRAY);
        glNormalPointer(GL_FLOAT, 0, n_normal);
        glColorPointer(3, GL_FLOAT, 0, m_colors);
        
        //escala
        int s = 16;
        for (int j=0; j<m_nx*m_ny*6; j++) {
            m_coord[j*3]*=s;
            m_coord[j*3+2]*=s;
        }
        //translacao
        for (int j=0; j<m_nx*m_ny*6; j++) {
            int tx = 8, ty = 5;
            m_coord[j*3] -= tx;
            m_coord[j*3+2] -= ty;
        }
        
        glVertexPointer(3, GL_FLOAT, 0, m_coord);
        
        glPushMatrix();
        
        glDrawArrays(GL_TRIANGLES, 0, m_nx*m_ny*6);
        
        glPopMatrix();
        
        glDisableClientState(GL_VERTEX_ARRAY);  // disable vertex arrays
        glDisableClientState(GL_COLOR_ARRAY);
        glDisableClientState(GL_NORMAL_ARRAY);
    }
};

class Sphere {
    // Create a sphere with the specified number of subdivisions
    // The sphere is centered at the origin with radius 1.0
public:
    int m_nx, m_ny;
    Sphere (int slices, int stack){
        this->m_nx = slices;
        this->m_ny = stack;
    }
    
    // Destroy a sphere
    ~Sphere ();
    
    // Draw a sphere
    void Draw (int x, int y){
        for (int j=0; j<m_ny; j++) {
            for (int i=0; i<m_nx; i++) {
                int k = Index(i, j);
                m_coord[18*k] = 0.5*cos( 2*PI*((float)i / m_nx)) * sin( PI*((float)j / m_nx) ) ;
                m_coord[18*k+1] = 0.5*cos( PI*((float)j / m_nx) );
                m_coord[18*k+2] = 0.5*sin( 2*PI*((float)i / m_nx) ) * sin( PI*((float)j / m_nx) ) ;
                
                m_coord[18*k+3] = 0.5*cos( 2*PI*(((float)i+1) / m_nx)) * sin( PI*(((float)j+1) / m_nx) ) ;
                m_coord[18*k+4] = 0.5*cos( PI*(((float)j+1) / m_nx) );
                m_coord[18*k+5] = 0.5*sin( 2*PI*(((float)i+1) / m_nx) ) * sin( PI*(((float)j+1) / m_nx) ) ;
                
                m_coord[18*k+6] = 0.5*cos( 2*PI*(((float)i+1) / m_nx)) * sin( PI*((float)j / m_nx) ) ;
                m_coord[18*k+7] = 0.5*cos( PI*((float)j / m_nx) );
                m_coord[18*k+8] = 0.5*sin( 2*PI*(((float)i+1) / m_nx) ) * sin( PI*((float)j / m_nx) ) ;
                
                
                m_coord[18*k+9] = 0.5*cos( 2*PI*((float)i / m_nx)) * sin( PI*((float)j / m_nx) ) ;
                m_coord[18*k+10] = 0.5*cos( PI*((float)j / m_nx) );
                m_coord[18*k+11] = 0.5*sin( 2*PI*((float)i / m_nx) ) * sin( PI*((float)j / m_nx) ) ;
                
                m_coord[18*k+12] = 0.5*cos( 2*PI*(((float)i+1) / m_nx)) * sin( PI*(((float)j+1) / m_nx) ) ;
                m_coord[18*k+13] = 0.5*cos( PI*(((float)j+1) / m_nx) );
                m_coord[18*k+14] = 0.5*sin( 2*PI*(((float)i+1) / m_nx) ) * sin( PI*(((float)j+1) / m_nx) ) ;
                
                m_coord[18*k+15] = 0.5*cos( 2*PI*((float)i / m_nx)) * sin( PI*(((float)j+1) / m_nx) ) ;
                m_coord[18*k+16] = 0.5*cos( PI*(((float)j+1) / m_nx) );
                m_coord[18*k+17] = 0.5*sin( 2*PI*((float)i / m_nx) ) * sin( PI*(((float)j+1) / m_nx) ) ;
            }
        }
        int k = 0;
        
        k = 0;
        for (int j=0; j<m_nx*m_ny*6; j++) {
            m_colors[k++]=1;
            m_colors[k++]=0;
            m_colors[k++]=0;
        }
        k = 0;
        for (int j=0; j<m_nx*m_ny*6; j++) {
            //printf("%d = %f %f %f\n",j,m_coord[j*3],m_coord[j*3+1],m_coord[j*3+2]);
        }
        
        // enble and specify pointers to vertex arrays
        glEnableClientState(GL_NORMAL_ARRAY);
        glEnableClientState(GL_COLOR_ARRAY);
        glEnableClientState(GL_VERTEX_ARRAY);
        glNormalPointer(GL_FLOAT, 0, m_coord);
        glColorPointer(3, GL_FLOAT, 0, m_colors);
        
        glVertexPointer(3, GL_FLOAT, 0, m_coord);
        
        //translacao
        for (int j=0; j<m_nx*m_ny*6; j++) {
            float tx=x-7.5, ty = y-5;
            m_coord[j*3]+=tx;
            m_coord[j*3+1]+=0.5;
            m_coord[j*3+2]+=ty;
        }
        
        glPushMatrix();
        
        glDrawArrays(GL_TRIANGLES, 0, m_nx*m_ny*6);
        
        glPopMatrix();
        
        glDisableClientState(GL_VERTEX_ARRAY);  // disable vertex arrays
        glDisableClientState(GL_COLOR_ARRAY);
        glDisableClientState(GL_NORMAL_ARRAY);
    }
};


// Initialization function
static void Init ()
{
    float white[4] = {1.0f,1.0f,1.0f,1.0f};
    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
    glEnable(GL_DEPTH_TEST);
    
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    glEnable(GL_COLOR_MATERIAL);
    glEnable(GL_NORMALIZE);
    glColorMaterial(GL_FRONT_AND_BACK,GL_AMBIENT_AND_DIFFUSE);
    glMaterialfv(GL_FRONT_AND_BACK,GL_SPECULAR,white);
    glMaterialf(GL_FRONT_AND_BACK,GL_SHININESS,50.0);
    glLightModeli(GL_LIGHT_MODEL_LOCAL_VIEWER,1);
}

// Draw scene
static void DrawScene ()
{
    // position light
    float lpos[4] = {0.0f,100.0f,0.0f,1.0f};
    glLightfv(GL_LIGHT0,GL_POSITION,lpos);
    
    Grid *g = new Grid(10,10);
    g->Draw();
    
    Sphere *s = new Sphere(20,20);
    for (int i=0; i<16; i+=2) {
        for (int j=0; j<16; j+=2) {
            s->Draw(i, j);
        }
    }
}

// Display callback
static void Display (void)
{
    int vp[4];
    glGetIntegerv(GL_VIEWPORT,vp);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(50.0f,(float)vp[2]/vp[3],1.0,100.0);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    
    // load manipulator matrix
    gluLookAt(0,7,15,0,0,0,0,1,0);
    
    
    // draw scene
    DrawScene();
    glutSwapBuffers();
}

// Reshape callback
static void Reshape (int w, int h)
{
    glViewport(0,0,w,h);
}

// Keyboard callback
static void Keyboard (unsigned char key, int x, int y)
{
    switch (key)
    {
        case 27: exit(0); break;
    }
}

// Main function
int main (int argc, char* argv[])
{
    // open GLUT
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_DEPTH | GLUT_RGB);
    glutInitWindowSize(800,600);
    
    // create window
    glutCreateWindow ("Spheres");
    glutReshapeFunc(Reshape);
    glutDisplayFunc(Display);
    glutKeyboardFunc(Keyboard);
    
    // initiate OpenGL context
    Init();
    
    // interact...
    glutMainLoop();
    return 0; 
}
