#ifndef LOCALIZER_H
#define LOCALIZER_H

#include "Maths.h"
#include "Config.h"

class Localizer {

public:
	virtual void setPosition(float x, float y, float orientation) { this->x = x; this->y = y; this->orientation = orientation; }
	virtual Math::Position getPosition() { return Math::Position(x, y, orientation); }
	virtual void move(float velocityX, float velocityY, float omega, float dt) = 0;
	virtual std::string getJSON() { return "null"; }

	float x;
	float y;
	float orientation;

};

#endif // LOCALIZER_H
