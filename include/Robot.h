#ifndef ROBOT_H
#define ROBOT_H

#include "Maths.h"
#include "Config.h"
#include "Odometer.h"
#include "ParticleFilterLocalizer.h"
#include "BallLocalizer.h"
#include "Vision.h"
#include "Tasks.h"
#include "AbstractCommunication.h"
#include "Command.h"
#include "PID.h"

#include <string>

class Wheel;
class Dribbler;
class Coilgun;
class Task;
class AbstractCommunication;
class OdometerLocalizer;

class Robot : public AbstractCommunication::Listener, public Command::Listener {

public:
	Robot(AbstractCommunication* com, CameraTranslator* frontCameraTranslator, CameraTranslator* rearCameraTranslator);
    ~Robot();

    void setup();

	void step(float dt, Vision::Results* visionResults);

    const Math::Position getPosition() const { return Math::Position(x, y, orientation);  }
    float getOrientation() const { return orientation; }
	float getVelocity() { return velocity; }
	bool isAccelerating() { return velocity > lastVelocity; }
	bool isBraking() { return velocity < lastVelocity; }
	float getOmega() { return omega; }
	float getTravelledDistance() { return travelledDistance; }
	float getTravelledRotation() { return travelledRotation; }
	float getTimeSincLastDroveBehindBall();
	bool isStalled();
	bool hasTasks() { return getCurrentTask() != NULL; }

    void setTargetDir(float x, float y, float omega = 0.0f);
    void setTargetDir(const Math::Angle& dir, float speed = 1.0f, float omega = 0.0f);
	void setTargetOmega(float omega) { targetOmega = omega; }
	void spinAroundDribbler(bool reverse = false, float period = Config::robotSpinAroundDribblerPeriod, float radius = Config::robotSpinAroundDribblerRadius, float forwardSpeed = Config::robotSpinAroundDribblerForwardSpeed);
    void setPosition(float x, float y, float orientation);
	void stop();
	void kick(int microseconds = Config::robotDefaultKickStrength);
	bool chipKick(float distance = 1.0f, bool lowerDribblerAfterwards = true);
	void clearTasks() { tasks.clear(); }
    void handleTasks(float dt);

	void lookAt(Object* object, float lookAtP = Config::lookAtP, bool stare = true);
	void lookAt(const Math::Angle& angle, float lookAtP = Config::lookAtP);
	void lookAtBehind(Object* object);
	void lookAtBehind(const Math::Angle& angle);
	void turnBy(float angle, float speed = 1.0f);
    void driveTo(float x, float y, float orientation, float speed = 1.0f);
    void driveFacing(float targetX, float targetY, float faceX, float faceY, float speed = 1.0f);
    void drivePath(const Math::PositionQueue positions, float speed = 1.0f);
	void driveBehindBall(float ballDistance, float targetAngle, float speed, float offsetDistance, float side);
	void stopRotation();
	void jumpAngle(float angle = 0.35f, float speed = 13.0f);
	void setTargetDirFor(float x, float y, float omega, float duration);

    void addTask(Task* task) { tasks.push_back(task); }
    Task* getCurrentTask();
    TaskQueue getTasks() { return tasks; }
	Odometer::Movement getMovement() { return movement; }
	const ParticleFilterLocalizer::Measurements& getMeasurements() const { return measurements; } // TODO Here?

	void handleCommunicationMessage(std::string message);
	bool handleCommand(const Command& cmd);

	std::string getJSON() { return json; }

	Wheel* wheelFL;
    Wheel* wheelFR;
    Wheel* wheelRL;
    Wheel* wheelRR;
	Dribbler* dribbler;
	Coilgun* coilgun;
	ParticleFilterLocalizer* robotLocalizer;
	BallLocalizer* ballLocalizer;
	OdometerLocalizer* odometerLocalizer;
	
private:
	void setupWheels();
	void setupDribbler();
	void setupCoilgun();
	void setupOdometer();
	void setupRobotLocalizer();
	void setupOdometerLocalizer();
	void setupBallLocalizer();
	void setupCameraFOV();
    void updateWheelSpeeds();
	void updateMeasurements();
	void updateBallLocalizer(Vision::Results* visionResults, float dt);
	void updateObjectsAbsoluteMovement(ObjectList* objectList, float robotX, float robotY, float robotOrientation, float dt);
	void updateAllObjectsAbsoluteMovement(Vision::Results* visionResults, float dt);
	void debugBallList(std::string name, std::stringstream& stream, BallLocalizer::BallList balls);
	void handleQueuedChipKickRequest();

    float x;
    float y;
    float orientation;
	float velocity;
	float lastVelocity;
	float omega;
	float travelledDistance;
	float travelledRotation;

	double lastCommandTime;
	double lastDriveBehindBallTime;
    float lastDt;
    float totalTime;
	bool coilgunCharged;

	bool chipKickRequested;
	bool requestedChipKickLowerDribbler;
	float requestedChipKickDistance;

    TaskQueue tasks;
	Math::Vector targetDir;
    float targetOmega;
	bool frameTargetSpeedSet;
	Math::Polygon cameraFOV;

	AbstractCommunication* com;
	CameraTranslator* frontCameraTranslator;
	CameraTranslator* rearCameraTranslator;
	Vision::Results* visionResults;
	Odometer* odometer;
	Odometer::Movement movement;
	ParticleFilterLocalizer::Measurements measurements;
	BallLocalizer::BallList visibleBalls;
	Math::Polygon currentCameraFOV;

	PID lookAtPid;

	std::string json;
};

#endif // ROBOT_H
