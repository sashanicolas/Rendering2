#include "camera.h"


Camera::Camera( Vector eye, Vector center, Vector up, float fov, float nearr, float farr, int w, int h )
{
	this->eye = eye;
	this->center = center;
	this->up = up;
	this->fov = fov;
	this->nearr = nearr;
	this->farr = farr;
	this->w = w;
	this->h = h;

	initialize();
}


Ray Camera::getRay(int x, int y)
{
	Vector vX = xe * ( b * ( (float) x / w - 0.5f ) );
	Vector vY = ye * ( a * ( (float) y / h - 0.5f ) );
	Vector vZ = ze * ( - df );

	return Ray( eye, vX + vY + vZ );
}


Vector Camera::getEye( )
{
	return eye;
}


int Camera::getWidth()
{
	return w;
}


int Camera::getHeight()
{
	return h;
}


void Camera::setWidth( int w )
{
	this->w = w;
}


void Camera::setHeight( int h )
{
	this->h = h;
}


void Camera::initialize()
{
	df = nearr;

	a = 2 * df * tan( fov / 2.0f );

	b = ( (float) w / h ) * a;

	ze = ( eye - center );
	ze = ze / ze.length();

	xe = up % ze;
	xe = xe / xe.length();

	ye = ze % xe;
}