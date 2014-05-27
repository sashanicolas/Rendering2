#include <GLUT/GLUT.h>
#include <iostream>
#include <stdio.h>
#include <cfloat>
#include "vector.h"
#include "scene.h"
#include "object.h"
#include "sphere.h"
#include "box.h"
#include "light.h"


#define INITIAL_WIDTH 600
#define INITIAL_HEIGHT 600

#define MAX_DEPTH 3

Scene* scene = NULL;

void createScene()
{
	Vector eye( 100, 40, 40 );
	Vector center( 0, 0, 0 );
	Vector up( 0, 1, 0);
	float fov = 90.0f;
	float nearr = 30.0f;
	float farr = 230.0f;
	int w = INITIAL_WIDTH;
	int h = INITIAL_HEIGHT;
    
	scene = new Scene();
	scene->createCamera( eye, center, up, fov, nearr, farr, INITIAL_WIDTH, INITIAL_HEIGHT );
	//scene->setAmbientColor( 0.5f, 0.5f, 0.5f );
	scene->setAmbientColor( 1.0f, 1.0f, 1.0f );
    
	Sphere* sphere = new Sphere( Vector(0, 20, 0), 25 );
	sphere->setColor( 0.0f, 0.0f, 1.0f, 1.0f );
	sphere->setDiffuseCoefficient( 0.3f );
	sphere->setSpecularCoefficient( 0.7f );
    
	LinedBox* box1 = new LinedBox( Vector( -80.0f, -50.0f, -50.0f ), Vector( 50.0f, -45.0f, 50.0f ) );
	box1->setColor( 0.7f, 0.7f, 0.0f );
	box1->setDiffuseCoefficient( 0.3f );
	box1->setSpecularCoefficient( 0.3f );
	box1->setReflectionCoefficient( 0.0f );
	//box1->setRefractiveIndex( 1.0f );
    
	LinedBox* box2 = new LinedBox( Vector( -80.0f, -50.0f, -60.0f ), Vector( 50.0f, 50.0f, -50.0f ) );
	box2->setColor( 0.7f, 0.7f, 0.0f );
	box2->setDiffuseCoefficient( 0.3f );
	box2->setSpecularCoefficient( 0.3f );
	box2->setReflectionCoefficient( 0.8f );
	//box1->setRefractiveIndex( 0.8f );
    
	LinedBox* box3 = new LinedBox( Vector( 49.8, -44, -50 ), Vector( 50, 35, 50 ) );
	box3->setColor( 0.7f, 0.7f, 0.0f, 0.5f );
	box3->setDiffuseCoefficient( 0.3f );
	box3->setSpecularCoefficient( 0.3f );
	box3->setReflectionCoefficient( 0.0f );
    
	scene->addObject( (Object *) sphere );
	scene->addObject( (Object *) box1 );
	scene->addObject( (Object *) box2 );
	//scene->addObject( (Object *) box3 );
    
	Light* light = new Light( );
    // Vector(60.0f, 120.0f, 40.0f), Color(1.0f, 1.0f, 1.0f)
    Color c(1.0f, 1.0f, 1.0f);
    light->setColor(c);
    Vector v(60.0f, 120.0f, 40.0f);
    light->setPosition(v);
	//Light* light2 = new Light( Vector(60.0f, 120.0f, -100.0f), Color(1.0f, 1.0f, 1.0f) );
	scene->addLight(light);
	//scene->addLight(light2);
}

void reshape(int w, int h)
{
	if (scene)
	{
		scene->getCamera()->setWidth( w );
		scene->getCamera()->setHeight( h );
	}
    
	glViewport(0, 0, w, h);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluOrtho2D(0, w, 0, h);
	glMatrixMode (GL_MODELVIEW);
	glLoadIdentity();
}

Color ambientColor( Object *object )
{
	Color color;
	Color ambientColor = scene->getAmbientColor( );
	Color objColor = object->getColor( );
	Color diffuseCf = object->getDiffuseCoefficient( );
    
	color.r = ambientColor.r * objColor.r * diffuseCf.r;
	color.g = ambientColor.g * objColor.g * diffuseCf.g;
	color.b = ambientColor.b * objColor.b * diffuseCf.b;
    
	return color;
}

Color diffuseColor( Light* light, Object* object, Ray ray, Vector interceptionPoint )
{
	Vector L = light->position - interceptionPoint;
	L = L / L.length();
    
	Vector normal = object->getNormal( interceptionPoint );
    
	float cosAng = L * normal;
	
	Color color = Color( -1.0f, -1.0f, -1.0f );
	if ( cosAng > 0 )
	{
		Color objColor = object->getColor();
		Color diffuseCf = object->getDiffuseCoefficient();
		color.r = light->color.r * diffuseCf.r * cosAng * objColor.r;
		color.g = light->color.g * diffuseCf.g * cosAng * objColor.g;
		color.b = light->color.b * diffuseCf.b * cosAng * objColor.b;
	}
    
	return color;
}

Color specularColor( Light* light, Object* object, Ray ray, Vector interceptionPoint )
{
	Vector L = light->position - interceptionPoint;
	L = L / L.length();
	
	Vector normal = object->getNormal( interceptionPoint );
	
	Vector r = ( normal * ( 2 * ( L * normal ) ) ) - L;
    
	Vector v = ray.origin - interceptionPoint;
	v = v / v.length( );
    
	int n = 12;
	float bright = pow( r * v, n );
    
	Color color;
	
	Color objColor = object->getColor();
	Color specularCf = object->getSpecularCoefficient();
	color.r = light->color.r * specularCf.r * bright;
	color.g = light->color.g * specularCf.g * bright;
	color.b = light->color.b * specularCf.b * bright;
    
	return color;
}

Color rayTracing(Ray ray, int depth = 1, Object* exceptionObject = NULL);

Color shade( Object* object, Ray ray, Vector interceptionPoint, int depth )
{
	//Por default, a cor ser· a resultante da iluminaÁ„o ambiente
	Color color = ambientColor( object );
	
	//Calcular a contribuiÁ„o de todas as luzes da cena para essa cor
	for (int i = 0; i < scene->getNumLights(); i++)
	{
		Light* light = scene->getLight( i );
        
		/* Sombra */
		
		//Vetor do ponto de interseÁ„o atÈ a luz
		Ray L( interceptionPoint, light->position - interceptionPoint );
        
		//Verifica se o vetor L atinge algum objeto antes de chegar ‡ luz
		//Se sim, ele n„o est· sendo o ponto n„o est· sendo iluminado por esta luz,
		//e ser· ump ponto de sombra
		bool occlusion = false;
		for ( int j = 0; j < scene->getNumObjects(); j++ )
		{
			Object* obj = scene->getObject( j );
            
			if ( obj != object )
			{
				//t: interseÁ„o do raio L e o objeto obj
				float t;
				if ( obj->intercepts( L, &t ) ) //H· um objeto ocluindo a luz
				{
					//Somente interessa a interseÁ„o que ocorre entre ponto e a luz
					//Se t < 0, o vetor intercepta o objeto antes
					if ( t > 0.0f )
					{
						occlusion = true;
						break;
					}
				}
			}
		}
		
		//Se n„o houve oclus„o, n„o h· sombra, ent„o a cor È calculada:
		if ( !occlusion )
		{
			//Calcula a cor resultante da reflex„o difusa
			Color dColor = diffuseColor( light, object, ray, interceptionPoint );
			
			//Calcula a cor resultante da reflex„o especulada (somente se houve reflex„o difusa)
			if ( dColor.r != -1.0f )
				color = color + dColor + specularColor( light, object, ray, interceptionPoint );
		}
	}
	
	//Normal do objeto
	Vector normal = object->getNormal( interceptionPoint );
	
	//Raio ray na direÁ„o oposta
	Vector opRay = ray.origin - interceptionPoint;
	opRay = opRay / opRay.length( );
    
	/* Reflex„o de outros objetos */
	float objReflection = object->getReflectionCoefficient( );
	if ( objReflection > 0.0f && depth + 1 <= MAX_DEPTH)
	{
		//Novo raio a ser lanÁado a partir do ponto de interseÁ„o
		Ray newRay( interceptionPoint, normal * ( 2 * ( opRay * normal ) ) - opRay );
        
		//Obtendo a cor refletida
		Color reflectedColor = rayTracing( newRay, depth + 1 );
        
		color = color * ( 1 - objReflection ) + reflectedColor * objReflection;
	}
    
	/* TransparÍncia */
	float opacity = object->getColor( ).a;
	if ( opacity < 1.0f && depth + 1 <= MAX_DEPTH)
	{
		Vector vT = normal * ( opRay * normal ) - opRay;
        
		float senAng = vT.length() * object->getRefractiveIndex( );
		float cosAng = sqrt( 1 - senAng * senAng );
        
		vT = vT / vT.length();
		Vector rT = vT * senAng - normal * cosAng;
        
		//Novo raio a ser lanÁado a partir do ponto de interseÁ„o
		Ray newRay( interceptionPoint, rT );
        
		//Obtendo a cor refletida
		Color behindColor = rayTracing( newRay, depth + 1, object );
        
		//if ( !(behindColor.r == 0.0f && behindColor.g == 0.0f && behindColor.b == 0.0f) )
        color = color * opacity + behindColor * ( 1 - opacity );
	}
	
	return color;
}

Color rayTracing( Ray ray, int depth, Object* exceptionObject )
{
	float minT = FLT_MAX;
    
	int indexObj = -1;
	for (int i = 0; i < scene->getNumObjects(); i++)
	{
		Object* obj = scene->getObject(i);
		
		if ( obj != exceptionObject )
		{
			float t;
			if ( obj->intercepts(ray, &t) )
			{
				if (t > 0.0f && t < minT)
				{
					minT = t;
					indexObj = i;
				}
			}
		}
	}
    
	Color color( 0.0f, 0.0f, 0.0f );
    
	if (minT < FLT_MAX)
	{
		Object *obj = scene->getObject(indexObj);
        
		Vector point = ray.origin + ray.direction * minT;
        
		color = shade( obj, ray, point, depth );
	}
    
	return color;
}

//FunÁ„o de desenho do colorChecker utilizando o OpenGL
void drawScene()
{
	glClearColor(0.7, 0.7, 0.7, 0.0);
	glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
	
	Camera* camera = scene->getCamera();
	int w = camera->getWidth();
	int h = camera->getHeight();
    
	glBegin(GL_POINTS);
	for (int y = 0; y < h; y++)
	{
		for (int x = 0; x < w; x++)
		{
			Ray ray = camera->getRay(x, y);
			
			Color color = rayTracing(ray);
			glColor3f(color.r, color.g, color.b);
			glVertex2f(x, y);
		}
	}
	glEnd();
    
	glutSwapBuffers();
}
// Keyboard callback
static void Keyboard (unsigned char key, int x, int y){
    switch (key){
        case 27: exit(0); break;
    }
}


//ConfiguraÁ„o da visualizaÁ„o do OpenGL
void Initialize() {
	glDisable(GL_DEPTH_TEST);
	glClearColor(0.0, 0.0, 0.0, 0.0);
	
	glClear(GL_COLOR_BUFFER_BIT);
	createScene();
	//glTranslatef(0.375, 0.375, 0);
}

int main(int argc, char**argv)
{
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA);
	glutInitWindowSize(INITIAL_WIDTH, INITIAL_HEIGHT);
	glutInitWindowPosition(200, 200);
	glutCreateWindow("Ray Tracing");
    glutKeyboardFunc(Keyboard);
	
	glutReshapeFunc(reshape);
	glutDisplayFunc(drawScene);
	
	Initialize();
	glutMainLoop();
	return 0;
}