#include "color.h"


Color::Color( )
{
	r = 0.0f;
	g = 0.0f;
	b = 0.0f;
	a = 1.0f;
}

Color::Color( float r, float g, float b, float a )
{
	this->r = r;
	this->g = g;
	this->b = b;
	this->a = a;
}

Color Color::operator+( const Color& c )
{
	return Color( r + c.r, g + c.g, b + c.b );
}

Color Color::operator*( float f )
{
	return Color( r * f, g * f, b * f );
}