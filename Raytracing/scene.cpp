#include "scene.h"


Scene::Scene()
{
}


Scene::~Scene()
{
	/*for ( int i = 0; i < objects.size(); i++ )
	{
		delete objects[i];
	}
	objects.erase();
	*/
}


void Scene::createCamera(Vector eye, Vector center, Vector up, float fov, float nearr, float farr, int w, int h )
{
	camera = new Camera( eye, center, up, fov, nearr, farr, w, h );
}


Camera* Scene::getCamera()
{
	return camera;
}


int Scene::getNumObjects()
{
	return objects.size();
}


void Scene::addObject(Object* o)
{
	objects.push_back(o);
}


Object* Scene::getObject(unsigned int index)
{
	if (index >= 0 && index < objects.size())
		return objects[index];
	return NULL;
}


int Scene::getNumLights()
{
	return lights.size();
}


void Scene::addLight(Light* l)
{
	lights.push_back(l);
}


Light* Scene::getLight(unsigned int index)
{
	if (index >= 0 && index < lights.size())
		return lights[index];
	return NULL;
}


void Scene::setAmbientColor( float r, float g, float b )
{
	ambientColor = Color( r, g, b );
}


Color Scene::getAmbientColor( )
{
	return ambientColor;
}