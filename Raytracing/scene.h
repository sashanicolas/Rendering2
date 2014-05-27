#ifndef SCENE_H
#define SCENE_H

#include <vector>
using namespace std;

#include "camera.h"
#include "object.h"
#include "light.h"

class Scene
{
public:
	Scene();
	virtual ~Scene();

	void createCamera( Vector eye, Vector center, Vector up, float fov, float near, float far, int w, int h );
	Camera* getCamera( );
	
	int getNumObjects( );
	void addObject( Object* o );
	Object* getObject( unsigned int index );
	
	int getNumLights( );
	void addLight( Light* l );
	Light* getLight( unsigned int index );

	void setAmbientColor( float r, float g, float b );
	Color getAmbientColor( );
	
private:
	Camera* camera;
	Color ambientColor;
	vector<Object *> objects;
	vector<Light *> lights;
};

#endif