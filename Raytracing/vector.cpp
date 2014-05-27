#include "vector.h"
#include <cmath>

Vector::Vector(float x, float y, float z) : x(x), y(y), z(z)
{

}


float Vector::length()
{
	return sqrt( x * x + y * y + z * z );
}


Vector Vector::operator+( const Vector& v )
{
	return Vector( x + v.x, y + v.y, z + v.z );
}


Vector Vector::operator-( const Vector& v )
{
	return Vector( x - v.x, y - v.y, z - v.z );
}


Vector Vector::operator*( float f )
{
	return Vector( x * f, y * f, z * f );
}


Vector Vector::operator/( float f )
{
	return Vector( x / f, y / f, z / f );
}


float Vector::operator*( const Vector& v)
{
	return x * v.x + y * v.y + z * v.z;
}


Vector Vector::operator%( const Vector& v)
{
	float uX = x;
	float uY = y;
	float uZ = z;

	float vX = v.x;
	float vY = v.y;
	float vZ = v.z;

	float sX = uY * vZ - uZ * vY;
	float sY = uZ * vX - uX * vZ;
	float sZ = uX * vY - uY * vX;

	return Vector( sX, sY, sZ );
}