#ifndef VECTOR_H
#define VECTOR_H


class Vector
{
public:
	Vector(){};
	Vector(float x, float y, float z);

	float length();
	Vector operator+( const Vector& v );
	Vector operator-( const Vector& v );
	Vector operator*( float f );
	Vector operator/( float f );
	float  operator*( const Vector& v);
	Vector operator%( const Vector& v);

	float x;
	float y;
	float z;
};

#endif