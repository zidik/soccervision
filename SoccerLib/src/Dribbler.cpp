#include "Dribbler.h"
#include "AbstractCommunication.h"
#include "Command.h"
#include "Coilgun.h"
#include "Util.h"
#include "Maths.h"

#include <iostream>

Dribbler::Dribbler(int id, AbstractCommunication* com, Coilgun* coilgun) : Wheel(id), com(com), coilgun(coilgun), ballDetected(false), everDetectedBall(false), isRaiseRequested(false), timeSinceRaised(0.0f), timeSinceLowered(0.0f), timeSinceLimitsApplied(0.0f), ballInDribblerTime(0.0), ballLostTime(-1.0f), stopRequestedTime(-1.0), lowerLimit(Config::robotDribblerNormalLowerLimit), upperLimit(Config::robotDribblerNormalUpperLimit), useChipKickLimitsMissedFrames(0) {
	applyLimits();
};

void Dribbler::prime() {
	setTargetSpeed(-Config::robotDribblerSpeed);
	//setTargetSpeed(-Config::robotDribblerSpeed / 2);

	stopRequestedTime = -1.0;
}

void Dribbler::start() {
	setTargetSpeed(-Config::robotDribblerSpeed);

	stopRequestedTime = -1.0;
}

void Dribbler::stop() {
	if (stopRequestedTime == -1.0) {
		stopRequestedTime = Util::millitime();
	}
}

void Dribbler::onKick() {
	ballLostTime = Config::dribblerBallLostThreshold; // make it large so the ball is not faked after kick
	ballDetected = false;
}

void Dribbler::setLowerLimit(int limit) {
	lowerLimit = (int)Util::limit((float)limit, 0.0f, 100.0f);

	applyLimits();
}

void Dribbler::setUpperLimit(int limit) {
	upperLimit = (int)Util::limit((float)limit, 0.0f, 100.0f);

	applyLimits();
}

void Dribbler::setLimits(int lower, int upper) {
	lowerLimit = (int)Util::limit((float)lower, 0.0f, 100.0f);
	upperLimit = (int)Util::limit((float)upper, 0.0f, 100.0f);

	applyLimits();
}

void Dribbler::useNormalLimits() {
	if (!isRaiseRequested) {
		return;
	}

	//std::cout << "! Now using normal dribbler limits" << std::endl;

	setLimits(Config::robotDribblerNormalLowerLimit, Config::robotDribblerNormalUpperLimit);

	isRaiseRequested = false;
}

void Dribbler::useChipKickLimits() {
	useChipKickLimitsMissedFrames = 0;

	if (isRaiseRequested) {
		return;
	}

	//std::cout << "! Now using chip-kick dribbler limits" << std::endl;

	setLimits(Config::robotDribblerChipKickLowerLimit, Config::robotDribblerChipKickUpperLimit);

	isRaiseRequested = true;
}

void Dribbler::applyLimits() {
	float min = (float)Config::robotDribblerLimitMin;
	float max = (float)Config::robotDribblerLimitMax;
	int servoLimitLower = (int)(Math::map((float)lowerLimit, 0.0f, 100.0f, min, max));
	int servoLimitUpper = (int)(min + max - Math::map((float)upperLimit, 0.0f, 100.0f, min, max));

	//std::cout << "! Setting servo limits to " << lowerLimit << "-" << upperLimit << " (" << servoLimitLower << "-" << servoLimitUpper << ")" << std::endl;
	
	com->send("servos:" + Util::toString(servoLimitLower) + ":" + Util::toString(servoLimitUpper));

	timeSinceLimitsApplied = 0.0f;
}
void Dribbler::step(float dt) {
	Wheel::step(dt);

	double delayStopPeriod = 0.1;

	if (stopRequestedTime != -1.0 && Util::duration(stopRequestedTime) >= delayStopPeriod) {
		setTargetOmega(0);
		//setTargetOmega(-Config::robotDribblerSpeed / 5);

		stopRequestedTime = -1.0;
	}

	if (ballDetected) {
		ballInDribblerTime += dt;

		if (ballInDribblerTime >= Config::ballInDribblerThreshold) {
			ballLostTime = -1.0f;
		}
	} else {
		if (everDetectedBall) {
			if (ballLostTime == -1.0f) {
				ballLostTime = dt;
			} else {
				ballLostTime += dt;
			}
		}

		if (ballLostTime >= Config::dribblerBallLostThreshold) {
			ballInDribblerTime = 0.0f;
		}
	}

	if (isRaiseRequested) {
		timeSinceRaised += dt;
		timeSinceLowered = 0.0f;
	} else {
		timeSinceLowered += dt;
		timeSinceRaised = 0.0f;
	}

	// apply limits just in case periodically
	if (timeSinceLimitsApplied > 1.0f) {
		applyLimits();
	}

	timeSinceLimitsApplied += dt;

	useChipKickLimitsMissedFrames++;

	/*if (useChipKickLimitsMissedFrames >= 2) {
		useNormalLimits();
	}*/

	//std::cout << "ballInDribblerTime: " << ballInDribblerTime << ", ballLostTime: " << ballLostTime << ", got ball: " << (gotBall() ? "yes" : "no") << std::endl;
}

bool Dribbler::isRaised() {
	return timeSinceRaised >= Config::robotDribblerMoveDuration;
}

bool Dribbler::isLowered() {
	return timeSinceLowered >= Config::robotDribblerMoveDuration;
}

bool Dribbler::gotBall(bool definitive) const {
	// don't show having ball after just having kicked
	if (coilgun->getTimeSinceLastKicked() < 0.1f) {
		//std::cout << "@ NO BALL AFTER KICK: " << coilgun->getTimeSinceLastKicked() << std::endl;

		return false;
	}

	// show ball in dribbler if it hasn't been lost for long
	if (!definitive && !ballDetected && ballLostTime != -1.0f && ballLostTime < Config::dribblerBallLostThreshold) {
		//std::cout << "@ FAKE GOT BALL, ACTUALLY LOST FOR: " << ballLostTime << std::endl;

		return true;
	}

	return ballDetected;
}

bool Dribbler::handleCommand(const Command& cmd) {
	Wheel::handleCommand(cmd);

	if (cmd.name == "ball") {
		if (cmd.parameters[0] == "1") {
			ballDetected = true;
			everDetectedBall = true;
		} else {
			ballDetected = false;
		}

		return true;
	}

	return false;
}
