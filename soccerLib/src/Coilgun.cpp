#include "Coilgun.h"
#include "AbstractCommunication.h"
#include "Util.h"
#include "Maths.h"
#include "Config.h"
#include "Command.h"

#include <iostream>

Coilgun::Coilgun(AbstractCommunication* com) : com(com), lastKickTime(0.0), lastChargeRequestTime(0.0), lastBdkickRequestTime(0.0), timeSinceLastVoltageReading(0.0f), voltage(0.0f), isKickingOnceGotBall(false), kickOnceGotBallMissedFrames(0) {

};

Coilgun::~Coilgun() {

}

void Coilgun::charge() {
	//std::cout << "! Requesting charging" << std::endl;

	com->send("charge");

	lastChargeRequestTime = Util::millitime();
}

void Coilgun::discharge() {
	com->send("discharge");
}

void Coilgun::kick(int microseconds) {
	if (lastKickTime == 0.0 || Util::duration(lastKickTime) >= Config::minKickInterval) {
		//std::cout << "! Kicking: " << microseconds << std::endl;

		// temporarily use chip-kick instead
		com->send("kick:" + Util::toString(microseconds));

		lastKickTime = Util::millitime();
	} else {
		std::cout << "! Another kick requested too soon" << std::endl;
	}
}

void Coilgun::chipKick(float distanceMeters) {
	int microseconds = getChipKickDurationByDistance(distanceMeters);

	//std::cout << "! Chip-kicking to distance of " << distanceMeters << " meters for " << microseconds << " microseconds" << std::endl;

	com->send("dkick:0:0:" + Util::toString(microseconds) + ":0");

	lastKickTime = Util::millitime();
}

int Coilgun::getChipKickDurationByDistance(float distanceMeters) {
	// https://docs.google.com/spreadsheets/d/1ARV-47Fz91zb0l3KWDKGF0UaeSvyiEqHsW1mHZ_QlIk/edit#gid=275766663
	float distanceCm = distanceMeters * 100.0f;

	// 400V: y = 1.103E-7x^4 - 9.764E-5x^3 + 0.014x ^ 2 + 11.672x + 908.503
	/*float a = 1.103E-7f;
	float b = 9.764E-5f;
	float c = 0.014f;
	float d = 11.672f;
	float e = 908.503f;

	return (int)(Math::pow(a * distanceCm, 4)
		- Math::pow(b * distanceCm, 3)
		+ Math::pow(c * distanceCm, 2)
		+ d * distanceCm
		+ e);*/

	// 300V: y = 3.405E-5x^3 - 0.028x ^ 2 + 16.707x + 1193.148
	float a = 3.405E-5f;
	float b = 0.028f;
	float c = 16.707f;
	float d = 1193.148f;

	return (int)(a * Math::pow(distanceCm, 3)
		- b * Math::pow(distanceCm, 2)
		+ c * distanceCm
		+ d);
}

float Coilgun::getTimeSinceLastKicked() {
	return (float)(Util::millitime() - lastKickTime);
}

void Coilgun::kickOnceGotBall(int mainDuration, int mainDelay, float chipDistance, int chipDelay) {
	kickOnceGotBallMissedFrames = 0;

	KickParameters kickParameters = KickParameters(mainDuration, mainDelay, chipDistance, chipDelay);

	if (isKickingOnceGotBall && kickParameters.areSameAs(kickOnceGotBallParameters)) {
		return;
	}

	kickOnceGotBallParameters = kickParameters;

	int chipDuration = kickOnceGotBallParameters.chipDistance > 0 ? getChipKickDurationByDistance(kickOnceGotBallParameters.chipDistance) : 0;

	std::string parametersStr = Util::toString(kickOnceGotBallParameters.mainDuration) + ":" + Util::toString(kickOnceGotBallParameters.mainDelay) + ":" + Util::toString(chipDuration) + ":" + Util::toString(kickOnceGotBallParameters.chipDelay);

	/*if (chipDistance > 0) {
		std::cout << "! Chip-kicking once got the ball to " << chipDistance << " meters" << std::endl;
	} else {
		std::cout << "! Kicking once got the ball: " << parametersStr << std::endl;
	}*/

	com->send("bdkick:" + parametersStr);

	isKickingOnceGotBall = true;
}

void Coilgun::cancelKickOnceGotBall(bool force) {
	if (!isKickingOnceGotBall && force != true) {
		return;
	}

	//std::cout << "! Cancelling kicking once got the ball" << std::endl;

	com->send("nokick");

	isKickingOnceGotBall = false;
	kickOnceGotBallParameters = KickParameters();
}

void Coilgun::step(float dt) {
	if (isLowVoltage() && Util::duration(lastChargeRequestTime) >= 1.0) {
		charge();
	}

	if (Util::duration(lastBdkickRequestTime) >= 1.0) {
		if (isKickingOnceGotBall) {
			int chipDuration = kickOnceGotBallParameters.chipDistance > 0 ? getChipKickDurationByDistance(kickOnceGotBallParameters.chipDistance) : 0;

			std::string parametersStr = Util::toString(kickOnceGotBallParameters.mainDuration) + ":" + Util::toString(kickOnceGotBallParameters.mainDelay) + ":" + Util::toString(chipDuration) + ":" + Util::toString(kickOnceGotBallParameters.chipDelay);

			com->send("bdkick:" + parametersStr);	
		} else {
			com->send("nokick");
		}

		lastBdkickRequestTime = Util::millitime();
	}

	timeSinceLastVoltageReading += dt;

	// request voltage readings 5 times per second
	if (timeSinceLastVoltageReading > 0.2f) {
		requestVoltageReading();

		timeSinceLastVoltageReading = 0.0f;
	}

	kickOnceGotBallMissedFrames++;

	if (kickOnceGotBallMissedFrames >= 2) {
		cancelKickOnceGotBall();
	}
}

void Coilgun::requestVoltageReading() {
	com->send("adc");
}

bool Coilgun::handleCommand(const Command& cmd) {
	if (cmd.name == "adc") {
		voltage = Util::toFloat(cmd.parameters[0]);

		return true;
	} else if (cmd.name == "kicked") {
		int chipDuration = kickOnceGotBallParameters.chipDistance > 0 ? getChipKickDurationByDistance(kickOnceGotBallParameters.chipDistance) : 0;

		//std::cout << "! Kicked @ main duration: " << kickOnceGotBallParameters.mainDuration << ", chip distance: " << kickOnceGotBallParameters.chipDistance << ", chip duration: " << chipDuration << std::endl;

		isKickingOnceGotBall = false;
		lastKickTime = Util::millitime();

		return true;
	}

	return false;
}
