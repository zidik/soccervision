#ifndef COILGUN_H
#define COILGUN_H

#include "Config.h"

class AbstractCommunication;
class Command;

class Coilgun {

public:
	Coilgun(AbstractCommunication* com);
	~Coilgun();

	struct KickParameters {
		KickParameters() : mainDuration(0), mainDelay(0), chipDistance(0), chipDelay(0) {}
		KickParameters(int mainDuration, int mainDelay, float chipDistance, int chipDelay) : mainDuration(mainDuration), mainDelay(mainDelay), chipDistance(chipDistance), chipDelay(chipDelay) {}

		bool areSameAs(KickParameters other) {
			return other.mainDuration == mainDuration
				&& other.mainDelay == mainDelay
				&& other.chipDistance == chipDistance
				&& other.chipDelay == chipDelay;
		}

		int mainDuration;
		int mainDelay;
		float chipDistance;
		int chipDelay;
	};

	void kick(int microseconds = Config::robotDefaultKickStrength);
	void chipKick(float distance);
	void kickOnceGotBall(int mainDuration, int mainDelay, float chipDistance, int chipDelay);
	void cancelKickOnceGotBall(bool force = false);
	bool willKickOnceGotBall() { return isKickingOnceGotBall;  }
	void charge();
	void discharge();
	bool isReady() { return false; } // TODO New communication
	bool isLowVoltage() { return voltage < Config::robotCoilgunLowVoltageThreshold;  }
	int getChipKickDurationByDistance(float distance);
	float getVoltage() { return voltage; }
	float getTimeSinceLastKicked();
	bool handleCommand(const Command& cmd);
	void step(float dt);

private:
	void requestVoltageReading();

	AbstractCommunication* com;
	double lastKickTime;
	double lastChargeRequestTime;
	double lastBdkickRequestTime;
	float timeSinceLastVoltageReading;
	float voltage;
	bool isKickingOnceGotBall;
	int kickOnceGotBallMissedFrames;
	KickParameters kickOnceGotBallParameters;

};

#endif //COILGUN_H