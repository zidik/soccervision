#ifndef DRIBBLER_H
#define DRIBBLER_H

#include "Config.h"
#include "Wheel.h"

class AbstractCommunication;
class Coilgun;

class Dribbler : public Wheel {

public:
	Dribbler(int id, AbstractCommunication* com, Coilgun* coilgun);

	void prime();
	void start();
	void stop();
	void onKick();
	bool isActive() const { return targetOmega > 0; }
	bool isRaised();
	bool isLowered();
	bool gotBall(bool definitive = false) const;
	bool handleCommand(const Command& cmd);
	float getBallInDribblerTime() { return ballInDribblerTime; }
	float getBallLostTime() { return ballLostTime; }
	int getLowerLimit() { return lowerLimit; }
	int getUpperLimit() { return upperLimit; }
	void setLowerLimit(int limit);
	void setUpperLimit(int limit);
	void setLimits(int lower, int upper);
	void useNormalLimits();
	void useChipKickLimits();
	void step(float dt);

private:
	void applyLimits();
	
	AbstractCommunication* com;
	Coilgun* coilgun;
	bool ballDetected;
	bool everDetectedBall;
	bool isRaiseRequested;
	float timeSinceRaised;
	float timeSinceLowered;
	float timeSinceLimitsApplied;
	float ballInDribblerTime;
	float ballLostTime;
	double stopRequestedTime;
	int lowerLimit;
	int upperLimit;
	int useChipKickLimitsMissedFrames;
};

#endif //DRIBBLER_H