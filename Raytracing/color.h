#ifndef COLOR_H
#define COLOR_H


class Color
{
public:
	Color( );

	Color( float r, float g, float b, float a = 1.0f );

	Color operator+( const Color& c );
	Color operator*( float f );

	float r;
	float g;
	float b;
	float a;
};


#endif