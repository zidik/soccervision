#include "Wheel.h"
#include "Maths.h"
#include "Command.h"
#include "Util.h"
#include "Config.h"

#include <iostream>

const float Wheel::pidFrequency = 60.0f;
const float Wheel::ticksPerRevolution = 64.0f * 18.75f;

Wheel::Wheel(int id) : id(id), targetOmega(0), filteredTargetOmega(0), realOmega(0), stallCounter(0) {
    
}

void Wheel::setTargetOmega(float omega) {
    targetOmega = omega;
}

void Wheel::setTargetSpeed(int speed) {
    setTargetOmega(speedToOmega((float)speed));
}

float Wheel::getTargetOmega() const {
    return targetOmega;
}

float Wheel::getFilteredTargetOmega() const {
	return filteredTargetOmega;
}

float Wheel::getTargetSpeed() const {
	//return omegaToSpeed(getTargetOmega());
	return omegaToSpeed(getFilteredTargetOmega());
}

float Wheel::getRealOmega() const {
    return realOmega;
}

bool Wheel::isStalled() {
	if (stallCounter > Config::robotWheelStalledThreshold) {
		return true;
	} else {
		return false;
	}
}

void Wheel::step(float dt) {
	float maxAccelerationPerSecond = Math::PI * 100.0f;

	if (filteredTargetOmega < targetOmega) {
		filteredTargetOmega = Math::min(filteredTargetOmega + maxAccelerationPerSecond * dt, targetOmega);
	} else if (filteredTargetOmega > targetOmega) {
		filteredTargetOmega = Math::max(filteredTargetOmega - maxAccelerationPerSecond * dt, targetOmega);
	}

	if (Math::abs(targetOmega) > Math::PI && Math::abs(targetOmega / realOmega) > 2.0f) {
		stallCounter++;
	} else {
		stallCounter = 0;
	}
}

bool Wheel::handleCommand(const Command& cmd) {
	if (cmd.name == "speeds") {
		if ((int)cmd.parameters.size() > id) {
			realOmega = speedToOmega((float)Util::toInt(cmd.parameters[id]));
		} else {
			std::cout << "- Invalid speeds info: " << cmd.name << " " << Util::toString(cmd.parameters) << std::endl;
		}

		return true;
	}

	return false;
}

/*std::string Wheel::getStateJSON() const {
    std::stringstream stream;

    stream << "{";
    stream << "\"targetOmega\":" << getTargetOmega() << ",";
    stream << "\"realOmega\":" << getRealOmega();
    stream << "}";

    return stream.str();
}*/

float Wheel::omegaToSpeed(float omega) {
    return Math::limit(omega / Math::TWO_PI * ticksPerRevolution / pidFrequency, 255);
}

float Wheel::speedToOmega(float speed) {
    return speed * pidFrequency / ticksPerRevolution * Math::TWO_PI;
}
