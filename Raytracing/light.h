#ifndef LIGHT_H
#define LIGHT_H

#include "color.h"
#include "vector.h"

class Light
{
public:
	Light( Vector& position, Color& color )
	{
		this->position = position;
		this->color = color;
	}
    Light(){
        
    }
    void setColor(Color& color){
        this->color = color;
    }
    void setPosition(Vector& position){
        this->position =position;
    }
	Vector position;
	Color color;
};


#endif;