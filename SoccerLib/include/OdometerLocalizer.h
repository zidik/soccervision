#ifndef ODOMETERLOCALIZER_H
#define ODOMETERLOCALIZER_H

#include "Localizer.h"

class OdometerLocalizer : public Localizer {

public:
    OdometerLocalizer();

	void move(float velocityX, float velocityY, float omega, float dt);
	std::string getJSON() { return json; }

private:
	std::string json;

};

#endif // ODOMETERLOCALIZER_H
