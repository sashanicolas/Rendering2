#ifndef CAMERA_H
#define CAMERA_H

#include <cmath>

#include "vector.h"
#include "ray.h"

class Vector;

class Camera
{
public:
	Camera( Vector eye, Vector center, Vector up, float fov, float nearr, float farr, int w, int h );

	Ray getRay(int x, int y);

	Vector getEye( );
	
	int getWidth( );
	int getHeight( );
	void setWidth( int w );
	void setHeight( int h );

private:
	//Calcula os par�metros intr�nsecos derivados
	void initialize();

	/* Par�metros extr�nsecos */
	Vector eye;
	Vector center;
	Vector up;

	/* Par�metros intr�nsecos */
	float nearr;
	float farr;
	int w;
	int h;
	float fov;

	/* Par�metros intr�nsecos derivados */
	float df;
	float a;
	float b;
	Vector xe;
	Vector ye;
	Vector ze;
};


#endif