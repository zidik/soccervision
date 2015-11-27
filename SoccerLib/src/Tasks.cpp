#include "Tasks.h"
#include "Robot.h"
#include "Util.h"

#include <iostream>

// TurnBy angle task
void TurnByTask::onStart(Robot& robot, float dt) {
	speed = Math::abs(speed);
    startTravelledRotation = robot.getTravelledRotation();
	currentTravelledRotation = startTravelledRotation;
	endTravelledRotation = startTravelledRotation + turnAngle;
	dir = (turnAngle > 0.0f ? 1.0f : -1.0f);
}

bool TurnByTask::onStep(Robot& robot, float dt) {
	currentTravelledRotation = robot.getTravelledRotation();

	if (
		(dir > 0.0f && currentTravelledRotation > endTravelledRotation)
		|| (dir < 0.0f && currentTravelledRotation < endTravelledRotation)
	) {
		return false;
	}

	diff = Math::abs(currentTravelledRotation - endTravelledRotation);

	// slow down at the end of the turn
	float applySpeed = Math::map(diff, 0.0f, Math::degToRad(30.0f), Math::degToRad(45.0f), speed) * dir;

    robot.setTargetDir(0.0f, 0.0f, applySpeed);

    return true;
}

void TurnByTask::onEnd(Robot& robot, float dt) {
    robot.stop();
}

float TurnByTask::getPercentage() {
    if (!started) {
        return 0.0f;
    }

    return diff * 100.0f / turnAngle;
}

std::string TurnByTask::toString() {
    return "TurnBy " + Util::toString(Math::round(Math::radToDeg(turnAngle), 1)) + " deg";
}

// DriveTo coordinates task
void DriveToTask::onStart(Robot& robot, float dt) {
    Math::Position pos = robot.getPosition();
	Math::Vector target(targetX, targetY);

    startX = pos.location.x;
    startY = pos.location.y;
    startOrientation = pos.orientation;

	startDistance = pos.location.distanceTo(target);
}

bool DriveToTask::onStep(Robot& robot, float dt) {
	Math::Position pos = robot.getPosition();
	Math::Vector target(targetX, targetY);

	currentDistance = pos.location.distanceTo(target);

	float currentOrientation = pos.orientation;
	float orientationDiff = Math::getAngleDiff(currentOrientation, targetOrientation);

	if (currentDistance <= positionThreshold && Math::abs(orientationDiff) < orientationThreshold) {
		return false;
	}

	float alpha = (currentDistance * 0.5f);
	if (alpha < 0.1f) {alpha = 0.1f;}
	float omega = orientationDiff / alpha;
    float useSpeed = speed;

    if (currentDistance < 0.2f) {
        useSpeed = speed * currentDistance * 5.0f;
    }

    Math::Vector globalDir = (target - pos.location).getScaledTo(useSpeed);
    Math::Vector localDir = globalDir.getRotated(-currentOrientation);

    //std::cout << "[" << Util::round(Math::radToDeg(currentOrientation)) << "] " << Util::round(globalDir.x, 1) << "x" << Util::round(globalDir.y) << " > " << Util::round(localDir.x, 1) << "x" << Util::round(localDir.y) << std::endl;
    //std::cout << "diff: " << Math::radToDeg(orientationDiff) << "; current: " << Math::radToDeg(currentOrientation) << "; to: " << Math::radToDeg(targetOrientation) << "; omega: " << omega << std::endl;
    //std::cout << "omega: " << omega << std::endl;

    if (omega < -3.0f) {
        omega = -3.0f;
    } else if (omega > 3.0f) {
        omega = 3.0f;
    }

    robot.setTargetDir(localDir.x, localDir.y, omega);

    return true;
}

void DriveToTask::onEnd(Robot& robot, float dt) {
    robot.setTargetDir(0, 0, 0);
}

float DriveToTask::getPercentage() {
    if (!started) {
        return 0.0f;
    }

    return 100.0f - (currentDistance * 100.0f / startDistance);
}

std::string DriveToTask::toString() {
    return "DriveTo " + Util::toString(targetX) + "x" + Util::toString(targetY) + " @ " + Util::toString(Math::round(Math::radToDeg(targetOrientation), 1)) + " deg";
}


// Follow path of coordinates task
void DrivePathTask::onStart(Robot& robot, float dt) {
    Math::Position pos = robot.getPosition();

    startX = pos.location.x;
    startY = pos.location.y;
    startOrientation = pos.orientation;
}

bool DrivePathTask::onStep(Robot& robot, float dt) {
    if (positions.size() == 0) {
        return false;
    }

    Math::Position currentPos = robot.getPosition();
    Math::Position targetPos = positions.front();

	float currentDistance = currentPos.location.distanceTo(targetPos.location);
    float orientationDiff = Math::getAngleDiff(currentPos.orientation, targetPos.orientation);

    if (currentDistance <= positionThreshold && Math::abs(orientationDiff) < orientationThreshold) {
        positions.pop();

        if (positions.size() == 0) {
            return false;
        } else {
            return onStep(robot, dt);
        }
    }

    float omega = orientationDiff / (currentDistance * 0.5f);
    float useSpeed = speed;

    if (currentDistance < 0.2f) {
        useSpeed = speed * currentDistance * 5.0f;
    }

    Math::Vector globalDir = (targetPos.location - currentPos.location).getScaledTo(useSpeed);
    Math::Vector localDir = globalDir.getRotated(-currentPos.orientation);

    if (omega < -3.0f) {
        omega = -3.0f;
    } else if (omega > 3.0f) {
        omega = 3.0f;
    }

    robot.setTargetDir(localDir.x, localDir.y, omega);

    return true;
}

void DrivePathTask::onEnd(Robot& robot, float dt) {
    robot.setTargetDir(0, 0, 0);
}

float DrivePathTask::getPercentage() {
    if (!started) {
        return 0.0f;
    }

    return 100.0f - (positions.size() * 100.0f / startPositionCount);
}

std::string DrivePathTask::toString() {
    if (positions.size() == 0) {
        return "DrivePath - empty";
    }

    Math::Position targetPos = positions.front();

    return "DrivePath " + Util::toString(startPositionCount - positions.size()) + "/" + Util::toString(startPositionCount) + " - " + Util::toString(targetPos.location.x) + "x" + Util::toString(targetPos.location.y) + " @ " + Util::toString(Math::round(Math::radToDeg(targetPos.orientation), 1)) + " deg";
}


// drive to coordinates facing some other coordinates
void DriveFacingTask::onStart(Robot& robot, float dt) {
    Math::Position pos = robot.getPosition();
	Math::Vector target(targetX, targetY);


    startX = pos.location.x;
    startY = pos.location.y;
    startOrientation = pos.orientation;

	startDistance = pos.location.distanceTo(target);
}

bool DriveFacingTask::onStep(Robot& robot, float dt) {
    Math::Position pos = robot.getPosition();
    Math::Vector target(targetX, targetY);

    currentDistance = pos.location.distanceTo(target);

    Math::Vector targetVec = (pos.location - target).getNormalized();
    float targetOrientation = Math::floatModulus(targetVec.getAngleBetween(Math::Vector(1.0f, 0.0f)) + Math::PI, Math::TWO_PI);
    float currentOrientation = pos.orientation;
    float orientationDiff = Math::getAngleDiff(currentOrientation, targetOrientation);

    if (currentDistance <= positionThreshold && Math::abs(orientationDiff) < orientationThreshold) {
        return false;
    }

    float omega = orientationDiff / (currentDistance * 0.5f);
    float useSpeed = speed;

    if (currentDistance < 0.2f) {
        useSpeed = speed * currentDistance * 5.0f;
    }

    Math::Vector globalDir = (target - pos.location).getScaledTo(useSpeed);
    Math::Vector localDir = globalDir.getRotated(-currentOrientation);

    //std::cout << "[" << Util::round(Math::radToDeg(currentOrientation)) << "] " << Util::round(globalDir.x, 1) << "x" << Util::round(globalDir.y) << " > " << Util::round(localDir.x, 1) << "x" << Util::round(localDir.y) << std::endl;
    //std::cout << "diff: " << Math::radToDeg(orientationDiff) << "; current: " << Math::radToDeg(currentOrientation) << "; to: " << Math::radToDeg(targetOrientation) << "; omega: " << omega << std::endl;
    //std::cout << "omega: " << omega << std::endl;

    if (omega < -3.0f) {
        omega = -3.0f;
    } else if (omega > 3.0f) {
        omega = 3.0f;
    }

    robot.setTargetDir(localDir.x, localDir.y, omega);

    return true;
}

void DriveFacingTask::onEnd(Robot& robot, float dt) {
    robot.setTargetDir(0, 0, 0);
}

float DriveFacingTask::getPercentage() {
    if (!started) {
        return 0.0f;
    }

    return 100.0f - (currentDistance * 100.0f / startDistance);
}

std::string DriveFacingTask::toString() {
    return "DriveFacing " + Util::toString(targetX) + "x" + Util::toString(targetY) + " facing " + Util::toString(faceX) + "x" + Util::toString(faceY);
}

// stops rotation
void StopRotationTask::onStart(Robot& robot, float dt) {
	startOmega = robot.getMovement().omega;
	startSign = startOmega > 0 ? 1 : 0;
	startTime = Util::millitime();
}

bool StopRotationTask::onStep(Robot& robot, float dt) {
	currentOmega = robot.getMovement().omega;
	diff = Math::abs(currentOmega - Config::rotationStoppedOmegaThreshold);
	int currentSign = currentOmega > 0 ? 1 : 0;

	if (
		Math::abs(currentOmega) < Config::rotationStoppedOmegaThreshold
		|| currentSign != startSign
		|| Util::duration(startTime) > 0.5f
	) {
		return false;
	}

	if (currentOmega > 0) {
		robot.setTargetDir(0, 0, -diff * Config::rotationCancelMultiplier);
	} else {
		robot.setTargetDir(0, 0, diff * Config::rotationCancelMultiplier);
	}

	return true;
}

void StopRotationTask::onEnd(Robot& robot, float dt) {
	robot.stop();
}

float StopRotationTask::getPercentage() {
    if (!started) {
        return 0.0f;
    }

	return 100.0f - (diff * 100.0f / Math::abs(startOmega));
}

std::string StopRotationTask::toString() {
    return "StopRotation";
}

// jumps by an angle
void JumpAngleTask::onStart(Robot& robot, float dt) {
	startOrientation = robot.getOrientation();
	targetOrientation = Math::floatModulus(startOrientation + angle, Math::TWO_PI);
	breaking = false;
}

bool JumpAngleTask::onStep(Robot& robot, float dt) {
	float currentOrientation = robot.getOrientation();
	float currentOmega = robot.getMovement().omega;
	diff = Math::abs(currentOrientation - targetOrientation);
	
	
	if (
		diff < 0.2f
		|| breaking
	) {
		breaking = true;

		if (
			Math::abs(currentOmega) < Config::rotationStoppedOmegaThreshold * 2
			|| currentOmega < 0.0f
		) {
			return false;
		} else {
			if (currentOmega > 0) {
				robot.setTargetDir(0, 0, -diff * Config::jumpAngleStopMultiplier);
			} else {
				robot.setTargetDir(0, 0, diff * Config::jumpAngleStopMultiplier);
			}
		}
	} else {
		robot.setTargetDir(0, 0, speed);
	}

	return true;
}

void JumpAngleTask::onEnd(Robot& robot, float dt) {
	robot.stop();
}

float JumpAngleTask::getPercentage() {
    if (!started) {
        return 0.0f;
    }

	return 100.0f - (diff * 100.0f / Math::abs(angle));
}

std::string JumpAngleTask::toString() {
    return "JumpAngle";
}


// drives for an duration
void DriveForTask::onStart(Robot& robot, float dt) {
	startTime = Util::millitime();
	endTime = startTime + duration;
}

bool DriveForTask::onStep(Robot& robot, float dt) {
	currentTime = Util::millitime();

	if (currentTime > endTime) {
		robot.stop();

		return false;
	}

	robot.setTargetDir(x, y, omega);

	return true;
}

void DriveForTask::onEnd(Robot& robot, float dt) {
	robot.stop();
}

float DriveForTask::getPercentage() {
    if (!started) {
        return 0.0f;
    }

	double timeRemaining = endTime - currentTime;

	return (float)(100.0 - (timeRemaining * 100.0 / duration));
}

std::string DriveForTask::toString() {
    return "Drive " + Util::toString(x) + "x" + Util::toString(y) + " @ " + Util::toString(omega) + " for " + Util::toString(duration) + "seconds";
}


// drive behind ball
void DriveBehindBallTask::onStart(Robot& robot, float dt) {
	startSpeed = robot.getSpeed();
	duration = 0.0f;
	arcRadius = offsetDistance / Math::cos(targetAngle - Math::PI);
	arcDistance = arcRadius * Math::TWO_PI / 4.0f; // quarter of a circle
	startTravelledDistance = robot.getTravelledDistance();
	travelledDistance = 0.0f;
	ballSideDistance = Math::sqrt(Math::pow(ballDistance, 2.0f) + Math::pow(offsetDistance, 2.0f));
	totalDistance = ballSideDistance + arcDistance;

	//std::cout << "@ Behind arc radius: " << arcRadius << ", arc distance: " << arcDistance << std::endl;
}

bool DriveBehindBallTask::onStep(Robot& robot, float dt) {
	duration += dt;

	travelledDistance = robot.getTravelledDistance() - startTravelledDistance; 

	if (travelledDistance > totalDistance) {
		robot.stop();
		
		return false;
	}

	// test stopping at ball side distance
	/*if (travelledDistance >= ballSideDistance) {
		robot.stop();

		return true;
	}*/

	float useSpeed = Math::map(travelledDistance, 0.0f, ballSideDistance, speed, speed / 2.0f);
	float sideSpeed = Math::map(travelledDistance, ballSideDistance, totalDistance, 0.0f, useSpeed);
	float forwardSpeed = useSpeed - sideSpeed;

	Math::Vector dirVector = Math::Vector::fromPolar(targetAngle, forwardSpeed/* * brakeMultiplier*/);

	dirVector.y += side * sideSpeed;

	robot.setTargetDir(dirVector.x, dirVector.y);

	return true;
}

void DriveBehindBallTask::onEnd(Robot& robot, float dt) {
	
}

float DriveBehindBallTask::getPercentage() {
    if (!started) {
        return 0.0f;
    }

	return travelledDistance * 100.0f / totalDistance;
}

std::string DriveBehindBallTask::toString() {
    return "Drive behind ball " + Util::toString(Math::round(travelledDistance)) + "/" + Util::toString(Math::round(totalDistance));
}