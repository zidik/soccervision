#ifndef TESTCONTROLLER_H
#define TESTCONTROLLER_H

#include "BaseAI.h"
#include "Vision.h"
#include "DebouncedButton.h"
#include "Util.h"
#include "PID.h"
#include "Config.h"

#include <map>
#include <string>

class TestController : public BaseAI {

public:
	enum TargetMode { LEFT = -1, INLINE = 0, RIGHT = 1, UNDECIDED = 2 };

	typedef std::map<std::string, std::string> Messages;
	typedef Messages::iterator MessagesIt;
	typedef std::map<int, std::string> Params;
	typedef std::vector<std::string> StateList;
	typedef StateList::iterator StateListIt;

	class State : public BaseAI::State {

	public:
		State(TestController* ai) : BaseAI::State(ai), ai(ai) {}

	protected:
		TestController* ai;

	};

	class ManualControlState : public State {

	public:
		ManualControlState(TestController* ai) : State(ai) {}
		void step(float dt, Vision::Results* visionResults, Robot* robot, float totalDuration, float stateDuration, float combinedDuration);

	};

	class WatchBallState : public State {

	public:
		WatchBallState(TestController* ai) : State(ai), kP(2.0f), kI(1.0f), kD(0.0015f), pid(kP, kI, kD, 0.016f) {}
		void onEnter(Robot* robot, Parameters parameters);
		void step(float dt, Vision::Results* visionResults, Robot* robot, float totalDuration, float stateDuration, float combinedDuration);

	private:
		PID pid;
		float kP;
		float kI;
		float kD;
	};

	class WatchGoalState : public State {

	public:
		WatchGoalState(TestController* ai) : State(ai), pid(5.5f, 1.0f, 0.005f, 0.016f) {}
		void onEnter(Robot* robot, Parameters parameters);
		void step(float dt, Vision::Results* visionResults, Robot* robot, float totalDuration, float stateDuration, float combinedDuration);

	private:
		PID pid;

	};

	class WatchGoalBehindState : public State {

	public:
		WatchGoalBehindState(TestController* ai) : State(ai) {}
		void step(float dt, Vision::Results* visionResults, Robot* robot, float totalDuration, float stateDuration, float combinedDuration);

	};

	class SpinAroundDribblerState : public State {

	public:
		SpinAroundDribblerState(TestController* ai) : State(ai) {}
		void step(float dt, Vision::Results* visionResults, Robot* robot, float totalDuration, float stateDuration, float combinedDuration);

	};

	class DriveToState : public State {

	public:
		DriveToState(TestController* ai) : State(ai), x(Config::fieldWidth / 2.0f), y(Config::fieldHeight / 2.0f), orientation(0.0f) {}
		void onEnter(Robot* robot, Parameters parameters);
		void step(float dt, Vision::Results* visionResults, Robot* robot, float totalDuration, float stateDuration, float combinedDuration);

		float x;
		float y;
		float orientation;

	};

	class TurnByState : public State {

	public:
		TurnByState(TestController* ai) : State(ai), angle(0.0f) {}
		void onEnter(Robot* robot, Parameters parameters);
		void step(float dt, Vision::Results* visionResults, Robot* robot, float totalDuration, float stateDuration, float combinedDuration);

		float angle;

	};

	class FindBallState : public State {

	public:
		FindBallState(TestController* ai) : State(ai), searchDir(1.0f), lastSearchTime(-1.0), lastTurnTime(-1.0), timeSinceLastSearch(-1.0), nearBothFrames(0), wasSearchingRecently(false), focusedOnGoal(false), queuedApproachGoal(false), useFetchBallDirectOnceFound(false) {}
		void onEnter(Robot* robot, Parameters parameters);
		void step(float dt, Vision::Results* visionResults, Robot* robot, float totalDuration, float stateDuration, float combinedDuration);

		float searchDir;

	private:
		double lastSearchTime;
		double lastTurnTime;
		double timeSinceLastSearch;
		int nearBothFrames;
		bool wasSearchingRecently;
		bool focusedOnGoal;
		bool queuedApproachGoal;
		bool useFetchBallDirectOnceFound;

	};

	class FetchBallFrontState : public State {

	public:
		FetchBallFrontState(TestController* ai) : State(ai), forwardSpeed(0.0f), startBrakingDistance(-1.0f), startBrakingVelocity(-1.0f), lastBallDistance(-1.0f), lastTargetAngle(0.0f), ballGoalOffsetAngleTooSmallFrames(0) {}
		void onEnter(Robot* robot, Parameters parameters);
		void step(float dt, Vision::Results* visionResults, Robot* robot, float totalDuration, float stateDuration, float combinedDuration);

	private:
		void reset(Robot* robot);

		float forwardSpeed;
		float startBrakingDistance;
		float startBrakingVelocity;
		float lastBallDistance;
		float lastTargetAngle;
		int ballGoalOffsetAngleTooSmallFrames;

	};

	class FetchBallDirectState : public State {

	public:
		FetchBallDirectState(TestController* ai) : State(ai), forwardSpeed(0.0f)/*, nearLineFrames(0), nearLine(false)*/ {}
		void onEnter(Robot* robot, Parameters parameters);
		void step(float dt, Vision::Results* visionResults, Robot* robot, float totalDuration, float stateDuration, float combinedDuration);

	private:
		float forwardSpeed;
		//int nearLineFrames;
		//bool nearLine;

	};

	class FetchBallBehindState : public State {

	public:
		FetchBallBehindState(TestController* ai) : State(ai), hadBall(false), forwardSpeed(0.0f), lastTargetAngle(0.0f), lostBallTime(-1.0), timeSinceLostBall(0.0), lostBallVelocity(0.0f), startBallDistance(-1.0f), lastBallDistance(-1.0f), targetMode(TargetMode::UNDECIDED), avgBallGoalDistance(10) {}
		void onEnter(Robot* robot, Parameters parameters);
		void step(float dt, Vision::Results* visionResults, Robot* robot, float totalDuration, float stateDuration, float combinedDuration);

	private:
		bool hadBall;
		bool reversePerformed;
		bool turnAroundPerformed;
		float forwardSpeed;
		float lastTargetAngle;
		float searchDir;
		double lostBallTime;
		double timeSinceLostBall;
		float lostBallVelocity;
		float startBallDistance;
		float lastBallDistance;
		TargetMode targetMode;
		Math::Avg avgBallGoalDistance;

	};

	class FetchBallNearState : public State {

	public:
		FetchBallNearState(TestController* ai) : State(ai), enterDistance(-1.0f), enterVelocity(0.0f), smallestForwardSpeed(-1.0f), useChipKick(false), switchToAim(false), chipKickDistance(0.0f), lastBallAngle(0.0f), ballInWayFrames(0), maxSideSpeed(1.5f),
			kP(4.0f), kI(1.0f), kD(0.0035f), pid(kP, kI, kD, 0.016f) {}
		void onEnter(Robot* robot, Parameters parameters);
		void onExit(Robot* robot);
		void step(float dt, Vision::Results* visionResults, Robot* robot, float totalDuration, float stateDuration, float combinedDuration);

	private:
		float enterDistance;
		float enterVelocity;
		float smallestForwardSpeed;
		bool useChipKick;
		bool switchToAim;
		float chipKickDistance;
		float lastBallAngle;
		float maxSideSpeed;
		int ballInWayFrames;

		PID pid;
		float kP;
		float kI;
		float kD;
	};

	class AimState : public State {

	public:
		AimState(TestController* ai) : State(ai), lastKickTime(-1.0), lastEscapeCornerTime(-1.0), avoidBallSide(TargetMode::UNDECIDED), searchGoalDir(0.0f), spinDuration(0.0f), reverseDuration(0.0f), avoidBallDuration(0.0f), nearLine(false), forceChipKick(false), escapeCornerPerformed(false), validKickFrames(0) {}
		void onEnter(Robot* robot, Parameters parameters);
		void step(float dt, Vision::Results* visionResults, Robot* robot, float totalDuration, float stateDuration, float combinedDuration);

	private:
		double lastKickTime;
		double lastEscapeCornerTime;
		float searchGoalDir;
		float spinDuration;
		float reverseDuration;
		TargetMode avoidBallSide;
		float avoidBallDuration;
		bool nearLine;
		bool forceChipKick;
		bool escapeCornerPerformed;
		int validKickFrames;
	};

	class DriveCircleState : public State {

	public:
		DriveCircleState(TestController* ai) : State(ai) {}
		void step(float dt, Vision::Results* visionResults, Robot* robot, float totalDuration, float stateDuration, float combinedDuration);

	private:
		static float getCircleTargetAngle(float start, float time, float period);

	};

	class AccelerateState : public State {

	public:
		AccelerateState(TestController* ai) : State(ai), forwardSpeed(0.0f) {}
		void onEnter(Robot* robot, Parameters parameters);
		void step(float dt, Vision::Results* visionResults, Robot* robot, float totalDuration, float stateDuration, float combinedDuration);

	private:
		float forwardSpeed;

	};

	class ReturnFieldState : public State {

	public:
		ReturnFieldState(TestController* ai) : State(ai), queuedApproachGoal(false) {}
		void onEnter(Robot* robot, Parameters parameters);
		void step(float dt, Vision::Results* visionResults, Robot* robot, float totalDuration, float stateDuration, float combinedDuration);

	private:
		bool queuedApproachGoal;

	};

	class EscapeCornerState : public State {

	public:
		EscapeCornerState(TestController* ai) : State(ai), startTravelledDistance(0.0f) {}
		void onEnter(Robot* robot, Parameters parameters);
		void step(float dt, Vision::Results* visionResults, Robot* robot, float totalDuration, float stateDuration, float combinedDuration);

	private:
		float startTravelledDistance;

	};

	class DriveHomeState : public State {

	public:
		DriveHomeState(TestController* ai) : State(ai), drivePerformed(false) {}
		void onEnter(Robot* robot, Parameters parameters);
		void step(float dt, Vision::Results* visionResults, Robot* robot, float totalDuration, float stateDuration, float combinedDuration);

	private:
		bool drivePerformed;

	};
	
	TestController(Robot* robot, AbstractCommunication* com);
	~TestController();

	void onEnter();
	void onExit();
	void reset();
	
	virtual void setState(std::string state);
	virtual void setState(std::string state, Parameters parameters);

    bool handleCommand(const Command& cmd);
	void handleTargetVectorCommand(const Command& cmd);
	void handleDribblerCommand(const Command& cmd);
	void handleAdjustDribblerLimitsCommand(const Command& cmd);
	void handleKickCommand(const Command& cmd);
	void handleChipKickCommand(const Command& cmd);
	void handleResetCommand();
	void handleToggleGoCommand();
	void handleToggleSideCommand();
	void handleDriveToCommand(const Command& cmd);
	void handleTurnByCommand(const Command& cmd);
	void handleParameterCommand(const Command& cmd);

	float getTargetAngle(float goalX, float goalY, float ballX, float ballY, float D, TargetMode targetMode = TargetMode::INLINE);
	float getChipKickDistance(Vision::BallInWayMetric ballInWayMetric, float goalDistance);
	bool shouldAvoidBallInWay(Vision::BallInWayMetric ballInWayMetric, float goalDistance);
	bool shouldManeuverBallInWay(Vision::BallInWayMetric ballInWayMetric, float goalDistance, bool isLowVoltage);
	Vision::Obstruction getGoalPathObstruction(float lifetime = 0.2f);

    void step(float dt, Vision::Results* visionResults);
	Side getTargetSide() { return targetSide; }
	void dbgs(std::string key, std::string value) { messages[key] = value; }
	void dbg(std::string key, int value) { messages[key] = Util::toString(value); }
	void dbg(std::string key, float value) { messages[key] = Util::toString(value); }
	void dbg(std::string key, double value) { messages[key] = Util::toString(value); }
	void dbg(std::string key, bool value) { messages[key] = value ? "true" : "false"; }
	std::string getJSON();

private:
	void setupStates();
	void updateVisionInfo(Vision::Results* visionResults);
	bool isRobotNearLine(Vision::Results* visionResults, bool ignoreCenterSample = false);
	bool isRobotInCorner(Vision::Results* visionResults);
	//bool isRobotNearGoal(float threshold = 0.8f);
	bool isRobotNearGoal(float distanceThreshold = 0.2f);
	bool isRobotNearTargetGoal(float threshold = 2.0f);
	bool wasNearLineLately(double threshold = 1.5);
	bool wasNearGoalLately(double threshold = 1.0);
	bool wasInCornerLately(double threshold = 1.5);
	void resetLastBall();
	void setLastBall(Object* ball);
	float getObjectClosestDistance(Vision::Results* visionResults, Object* object);
	Object* getLastBall(Dir dir = Dir::ANY);

	Vision::Results* visionResults;

	DebouncedButton toggleGoBtn;
	DebouncedButton toggleSideBtn;
	DebouncedButton resetBtn;

	Side targetSide;
	float speedMultiplier;
	float manualSpeedX;
	float manualSpeedY;
	float manualOmega;
	int manualDribblerSpeed;
	int manualKickStrength;
	float blueGoalDistance;
	float yellowGoalDistance;
	float lastTargetGoalAngle;
	float lastClosestGoalDistance;
	float lastTargetGoalDistance;
	Math::Avg lastClosestGoalDistanceAvg;
	Math::Avg lastTargetGoalDistanceAvg;
	Vision::ColorDistance whiteDistance;
	Vision::ColorDistance blackDistance;
	double lastTurnAroundTime;
	int framesRobotOutFront;
	int framesRobotOutRear;
	bool isRobotOutFront;
	bool isRobotOutRear;
	bool isNearLine;
	bool isNearGoal;
	bool isInCorner;
	bool isBallInWay;
	bool isAvoidingBallInWay;
	int inCornerFrames;
	int nearLineFrames;
	int nearGoalFrames;
	int visibleBallCount;

	double lastCommandTime;
	double lastBallTime;
	double lastNearLineTime;
	double lastNearGoalTime;
	double lastInCornerTime;
	double lastGoalObstructedTime;

	Object* lastBall;
	Vision::Obstruction lastGoalPathObstruction;

	Params parameters;
	Messages messages;
	StateList stateChanges;
};

#endif // TESTCONTROLLER_H
