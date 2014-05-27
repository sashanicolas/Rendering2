#include "sphere.h"

Sphere::Sphere( Vector center, float r )
{
	this->center = center;
	this->ray = r;
}

bool Sphere::intercepts( Ray r, float* t )
{
	float a = r.direction *  r.direction;
	float b = 2 * ( r.direction * ( r.origin - center ) );
	float c = ( ( r.origin - center ) * ( r.origin - center ) ) - ( ray * ray );

	float delta = ( b * b ) - ( 4 * a * c );
	if ( delta > 0 )
	{
		float t1 = ( -b + sqrt( delta ) ) / (2 * a);
		float t2 = ( -b - sqrt( delta ) ) / (2 * a);

		if ( t1 > 0 || t2 > 0 )
		{
			if ( t1 > 0 )
				(*t) = t1;
			if (t2 > 0 && t2 < t1)
				(*t) = t2;

			return true;
		}
	}

	return false;
}

Vector Sphere::getNormal( Vector point )
{
	Vector normal = point - center;
	normal = normal / normal.length();
	return normal;
}