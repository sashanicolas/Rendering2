#ifndef SPHERE_H
#define SPHERE_H

#include <cmath>

#include "object.h"
#include "vector.h"
#include "ray.h"

class Sphere : public Object
{
public:
	Sphere( Vector center, float r );

	bool intercepts( Ray r, float* point );
	Vector getNormal( Vector point );

private:
	Vector center;
	float ray;
};


#endif


