#include "box.h"

LinedBox::LinedBox( Vector minPoint, Vector maxPoint )
{
	this->minPoint = minPoint;
	this->maxPoint = maxPoint;
}

bool LinedBox::intercepts( Ray r, float* t )
{
	/*Testar a interceção com cada um dos seis planos da caixa*/

	float xMin = minPoint.x;
	float xMax = maxPoint.x;
	float yMin = minPoint.y;
	float yMax = maxPoint.y;
	float zMin = minPoint.z;
	float zMax = maxPoint.z;

	Vector point;

	if ( r.direction.x < 0 ) //O raio intercepta o plano xMax
	{
		(*t) = ( xMax - r.origin.x ) / r.direction.x;
		point = r.origin + r.direction * (*t);

		if ( point.y >= yMin && point.y <= yMax && point.z >= zMin && point.z <= zMax )
		{
			lastCalcNormal = Vector( 1, 0, 0 );
			return true;
		}
	}
	
	else //O raio intercepta o plano xMin
	{
		(*t) = ( xMin - r.origin.x ) / r.direction.x;
		point = r.origin + r.direction * (*t);

		if ( point.y >= yMin && point.y <= yMax && point.z >= zMin && point.z <= zMax )
		{
			lastCalcNormal = Vector( -1, 0, 0 );
			return true;
		}
	}
	
	if ( r.direction.y < 0 ) //O raio intercepta o plano yMax
	{
		(*t) = ( yMax - r.origin.y ) / r.direction.y;
		point = r.origin + r.direction * (*t);

		if ( point.x >= xMin && point.x <= xMax && point.z >= zMin && point.z <= zMax )
		{
			lastCalcNormal = Vector( 0, 1, 0 );
			return true;
		}
	}
	
	else //O raio intercepta o plano yMin
	{
		(*t) = ( yMin - r.origin.y ) / r.direction.y;
		point = r.origin + r.direction * (*t);

		if ( point.x >= xMin && point.x <= xMax && point.z >= zMin && point.z <= zMax )
		{
			lastCalcNormal = Vector( 0, -1, 0 );
			return true;
		}
	}

	if ( r.direction.z < 0 ) //O raio intercepta o plano zMax
	{
		(*t) = ( zMax - r.origin.z ) / r.direction.z;
		point = r.origin + r.direction * (*t);

		if ( point.x >= xMin && point.x <= xMax && point.y >= yMin && point.y <= yMax )
		{
			lastCalcNormal = Vector( 0, 0, 1 );
			return true;
		}
	}

	else //O raio intercepta o plano zMin
	{
		(*t) = ( zMin - r.origin.z ) / r.direction.z;
		point = r.origin + r.direction * (*t);

		if ( point.x >= xMin && point.x <= xMax && point.y >= yMin && point.y <= yMax )
		{
			lastCalcNormal = Vector( 0, 0, -1 );
			return true;
		}
	}

	return false;
}

Vector LinedBox::getNormal( Vector point )
{
	return lastCalcNormal;
}