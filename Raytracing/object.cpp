#include "object.h"

Object::Object( )
{
	color = Color( 0.0f, 0.0f, 0.0f );
	diffuseCf = Color( 0.0f, 0.0f, 0.0f );
	specularCf = Color( 0.0f, 0.0f, 0.0f );
	reflectionCf = 0.0f;
	refractiveIndex = 1.0f;
}


	
void Object::setColor( float r, float g, float b, float a )
{
	color.r = r;
	color.g = g;
	color.b = b;
	color.a = a;
}



void Object::setDiffuseCoefficient( float dC )
{
	diffuseCf.r = dC;
	diffuseCf.g = dC;
	diffuseCf.b = dC;
}
	


void Object::setDiffuseCoefficient( float dcR, float dcG, float dcB )
{
	diffuseCf.r = dcR;
	diffuseCf.g = dcG;
	diffuseCf.b = dcB;
}



void Object::setSpecularCoefficient( float sC )
{
	specularCf.r = sC;
	specularCf.g = sC;
	specularCf.b = sC;
}



void Object::setSpecularCoefficient( float scR, float scG, float scB )
{
	specularCf.r = scR;
	specularCf.g = scG;
	specularCf.b = scB;
}



Color Object::getColor( )
{
	return color;
}
	

void Object::getColor( float* r, float* g, float* b )
{
	(*r) = color.r;
	(*g) = color.g;
	(*b) = color.b;
}



Color Object::getDiffuseCoefficient()
{
	return diffuseCf;
}



Color Object::getSpecularCoefficient()
{
	return specularCf;
}



void Object::setReflectionCoefficient( float rcF )
{
	reflectionCf = rcF;
}



float Object::getReflectionCoefficient( )
{
	return reflectionCf;
}



void Object::setRefractiveIndex( float rfIndex )
{
	refractiveIndex = rfIndex;
}



float Object::getRefractiveIndex( )
{
	return refractiveIndex;
}