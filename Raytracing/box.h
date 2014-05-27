#ifndef BOX_H
#define BOX_H

#include "object.h"

class LinedBox : public Object
{
public:
	LinedBox( Vector minPoint, Vector maxPoint );

	bool intercepts( Ray r, float* point );
	Vector getNormal( Vector point );

private:
	Vector minPoint;
	Vector maxPoint;

	Vector lastCalcNormal;
};


#endif;