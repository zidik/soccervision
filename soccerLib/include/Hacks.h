#ifndef HACKS_H
#define HACKS_H
#include "Maths.h"
#include "CameraTranslator.h"

inline void HACKFLIP(Math::Vector &vector)
{
	//TODO : PLEASE REMOVE THIS HACK
	float temp = vector.x;
	vector.x = vector.y;
	vector.y = temp;
}

inline void HACKFLIP(CameraTranslator::CameraPosition &vector)
{
	//TODO : PLEASE REMOVE THIS HACK
	float temp = vector.x;
	vector.x = vector.y;
	vector.y = temp;
}

#endif