#include "OdometerLocalizer.h"
#include "Maths.h"
#include "Util.h"

#include <string>
#include <sstream>

OdometerLocalizer::OdometerLocalizer() {
	x = Config::fieldWidth / 2.0f;
	y = Config::fieldHeight / 2.0f;
	orientation = 0.0f;
	json = "null";
}

void OdometerLocalizer::move(float velocityX, float velocityY, float omega, float dt) {
	orientation = Math::floatModulus(orientation + omega * dt, Math::TWO_PI);
    x += (velocityX * Math::cos(orientation) - velocityY * Math::sin(orientation)) * dt;
    y += (velocityX * Math::sin(orientation) + velocityY * Math::cos(orientation)) * dt;

	//Util::confineField(x, y);

	std::stringstream stream;

    stream << "{";
	stream << "\"x\": " << x << ",";
	stream << "\"y\": " << y << ",";
	stream << "\"orientation\": " << orientation;
	stream << "}";

    json = stream.str();
}
