#ifndef OBJECT_H
#define OBJECT_H

#include "color.h"
#include "vector.h"
#include "ray.h"

class Object
{
public:
	Object( );

	virtual bool intercepts( Ray r, float* point ) = 0;
	virtual Vector getNormal( Vector point ) = 0;
	
	void setColor( float r, float g, float b, float a = 1.0f );
	void setDiffuseCoefficient( float dC );
	void setDiffuseCoefficient( float dcR, float dcG, float dcB );
	void setSpecularCoefficient( float sC );
	void setSpecularCoefficient( float scR, float scG, float scB );

	Color getColor( );
	void getColor( float* r, float* g, float* b );
	Color getDiffuseCoefficient();
	Color getSpecularCoefficient();

	void setReflectionCoefficient( float rcF );
	float getReflectionCoefficient( );

	void setRefractiveIndex( float rfIndex );
	float getRefractiveIndex( );


private:
	Color color;
	Color diffuseCf;
	Color specularCf;
	float reflectionCf;
	float refractiveIndex;
};

#endif