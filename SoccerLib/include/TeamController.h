#ifndef TEAMCONTROLLER_H
#define TEAMCONTROLLER_H

#include "TestController.h"

class TeamController : public TestController {

public:
	enum GameSituation { UNKNOWN = -1, KICKOFF = 0, INDIRECTFREEKICK = 1, DIRECTFREEKICK = 2, GOALKICK = 3, THROWIN = 4, CORNERKICK = 5, PENALTY = 6, PLACEDBALL = 7, ENDHALF = 8 };
	enum TeamInPossession { NOONE = -1, ENEMY = 0, FRIENDLY = 1 };

	class State : public BaseAI::State {

	public:
		State(TeamController* ai) : BaseAI::State(ai), ai(ai) {}

	protected:
		TeamController* ai;

	};

	//This state detects if ball has been kicked starting with a stationary robot
	class WaitForKickState : public State {

	public:
		WaitForKickState(TeamController* ai) : State(ai), startingBallPos(-1000.0f, -1000.0f) {}
		void onEnter(Robot* robot, Parameters parameters);
		void step(float dt, Vision::Results* visionResults, Robot* robot, float totalDuration, float stateDuration, float combinedDuration);

	private:
		Math::Vector startingBallPos;
		std::string nextState;
	};

	//State for taking kickoff
	class TakeKickoffState : public State {

	public:
		TakeKickoffState(TeamController* ai) : State(ai) {}
		void onEnter(Robot* robot, Parameters parameters);
		void step(float dt, Vision::Results* visionResults, Robot* robot, float totalDuration, float stateDuration, float combinedDuration);
	};

	//Take direct free kick
	class TakeFreeKickDirectState : public State {

	public:
		TakeFreeKickDirectState(TeamController* ai) : State(ai) {}
		void onEnter(Robot* robot, Parameters parameters);
		void step(float dt, Vision::Results* visionResults, Robot* robot, float totalDuration, float stateDuration, float combinedDuration);
	private:
		bool areaLocked;
		Part lockedArea;
	};

	//take indirect free kick
	class TakeFreeKickIndirectState : public State {

	public:
		TakeFreeKickIndirectState(TeamController* ai) : State(ai) {}
		void onEnter(Robot* robot, Parameters parameters);
		void step(float dt, Vision::Results* visionResults, Robot* robot, float totalDuration, float stateDuration, float combinedDuration);

	};

	//take goalkick
	class TakeGoalkickState : public State {

	public:
		TakeGoalkickState(TeamController* ai) : State(ai) {}
		void onEnter(Robot* robot, Parameters parameters);
		void step(float dt, Vision::Results* visionResults, Robot* robot, float totalDuration, float stateDuration, float combinedDuration);

	};

	//take throwin
	class TakeThrowInState : public State {

	public:
		TakeThrowInState(TeamController* ai) : State(ai) {}
		void onEnter(Robot* robot, Parameters parameters);
		void step(float dt, Vision::Results* visionResults, Robot* robot, float totalDuration, float stateDuration, float combinedDuration);

	};

	//take cornerkick
	class TakeCornerKickState : public State {

	public:
		TakeCornerKickState(TeamController* ai) : State(ai) {}
		void onEnter(Robot* robot, Parameters parameters);
		void step(float dt, Vision::Results* visionResults, Robot* robot, float totalDuration, float stateDuration, float combinedDuration);
	};

	//take penalty
	class TakePenaltyState : public State {

	public:
		TakePenaltyState(TeamController* ai) : State(ai) {}
		void onEnter(Robot* robot, Parameters parameters);
		void step(float dt, Vision::Results* visionResults, Robot* robot, float totalDuration, float stateDuration, float combinedDuration);
	};

	//This state is for the goalkeeper
	class DefendGoalState : public State {

	public:
		DefendGoalState(TeamController* ai) : State(ai), kP(2.5f), kI(0.75f), kD(0.00275f), pid(kP, kI, kD, 0.016f), pidUpdateCounter(0) {}
		void onEnter(Robot* robot, Parameters parameters);
		void step(float dt, Vision::Results* visionResults, Robot* robot, float totalDuration, float stateDuration, float combinedDuration);
	private:
		PID pid;
		float kP;
		float kI;
		float kD;
		char pidUpdateCounter;
	};

	//For intercepting a moving ball
	class InterceptBallState : public State {

	public:
		InterceptBallState(TeamController* ai) : State(ai), ballWasSeen(false) {}
		void onEnter(Robot* robot, Parameters parameters);
		void step(float dt, Vision::Results* visionResults, Robot* robot, float totalDuration, float stateDuration, float combinedDuration);

	private:
		bool ballWasSeen;
	};

	//Find the ball
	class FindBallState : public State {

	public:
		FindBallState(TeamController* ai) : State(ai) {}
		void onEnter(Robot* robot, Parameters parameters);
		void step(float dt, Vision::Results* visionResults, Robot* robot, float totalDuration, float stateDuration, float combinedDuration);

	private:
		std::string nextState;
	};

	//Find any object given in as a parameter
	class FindObjectState : public State {

	public:
		FindObjectState(TeamController* ai) : State(ai) {}
		void onEnter(Robot* robot, Parameters parameters);
		void step(float dt, Vision::Results* visionResults, Robot* robot, float totalDuration, float stateDuration, float combinedDuration);

	private:
		std::string nextState;
		std::string lastState;
		std::string targetType;
	};

	//Find the ball while defending the goal
	class FindBallGoalkeeperState : public State {

	public:
		FindBallGoalkeeperState(TeamController* ai) : State(ai) {}
		void onEnter(Robot* robot, Parameters parameters);
		void step(float dt, Vision::Results* visionResults, Robot* robot, float totalDuration, float stateDuration, float combinedDuration);
	};

	//Fetch the ball @front
	class FetchBallFrontState : public State {

	public:
		FetchBallFrontState(TeamController* ai) : State(ai), kP(3.0f), kI(1.0f), kD(0.0035f), pid(kP, kI, kD, 0.016f), pidUpdateCounter(0) {}
		void onEnter(Robot* robot, Parameters parameters);
		void step(float dt, Vision::Results* visionResults, Robot* robot, float totalDuration, float stateDuration, float combinedDuration);
	private:
		float maxSideSpeed;

		PID pid;
		float kP;
		float kI;
		float kD;
		char pidUpdateCounter;

		enum FetchStyle {
			DIRECT = 0,
			DEFENSIVE = 1,
			OFFENSIVE = 2
		};

		FetchStyle fetchStyle;
		std::string lastState;
		std::string nextState;
	};

	//Fetch the ball @rear
	class FetchBallRearState : public State {

	public:
		FetchBallRearState(TeamController* ai) : State(ai) {}
		void onEnter(Robot* robot, Parameters parameters);
		void step(float dt, Vision::Results* visionResults, Robot* robot, float totalDuration, float stateDuration, float combinedDuration);
	};

	//Drive in front of own goal
	class DriveToOwnGoalState : public State {

	public:
		DriveToOwnGoalState(TeamController* ai) : State(ai) {}
		void onEnter(Robot* robot, Parameters parameters);
		void step(float dt, Vision::Results* visionResults, Robot* robot, float totalDuration, float stateDuration, float combinedDuration);
	};

	//Aim for a kick
	class AimKickState : public State {

	public:
		AimKickState(TeamController* ai) : State(ai) {}
		void onEnter(Robot* robot, Parameters parameters);
		void step(float dt, Vision::Results* visionResults, Robot* robot, float totalDuration, float stateDuration, float combinedDuration);
	private:
		std::string lastState;
		std::string nextState;
		std::string targetType;
		std::string kickType;
		bool canMoveWithBall;
		int validCount;
		bool areaLocked;
		Part lockedArea;
		bool chipRequested;

		std::vector<float> targetAngleBuffer;

	};

	//Pass the ball
	class PassBallState : public State {

	public:
		PassBallState(TeamController* ai) : State(ai) {}
		void onEnter(Robot* robot, Parameters parameters);
		void step(float dt, Vision::Results* visionResults, Robot* robot, float totalDuration, float stateDuration, float combinedDuration);
	};

	//Receive passed ball
	class GetPassState : public State {

	public:
		GetPassState(TeamController* ai) : State(ai) {}
		void onEnter(Robot* robot, Parameters parameters);
		void step(float dt, Vision::Results* visionResults, Robot* robot, float totalDuration, float stateDuration, float combinedDuration);
	};

	//for slowly approaching the ball during kickoffs and what not
	class ApproachBallState : public State {

	public:
		ApproachBallState(TeamController* ai) : State(ai), kP(4.0f), kI(1.0f), kD(0.0035f), pid(kP, kI, kD, 0.016f), pidUpdateCounter(0) {}
		void onEnter(Robot* robot, Parameters parameters);
		void step(float dt, Vision::Results* visionResults, Robot* robot, float totalDuration, float stateDuration, float combinedDuration);
	private:
		std::string lastState;
		std::string nextState;
		std::string targetType;
		std::string kickType;
		bool kickImmediately;
		int validCount;
		bool areaLocked;
		Part lockedArea;

		float maxSideSpeed;

		PID pid;
		float kP;
		float kI;
		float kD;
		char pidUpdateCounter;

		std::vector<float> targetAngleBuffer;
	};

	//for driving to ball for taking a kick
	class GoToBallState : public State {

	public:
		GoToBallState(TeamController* ai) : State(ai) {}
		void onEnter(Robot* robot, Parameters parameters);
		void step(float dt, Vision::Results* visionResults, Robot* robot, float totalDuration, float stateDuration, float combinedDuration);
	private:
		std::string lastState;
		std::string nextState;
		std::string targetType;
		std::string kickType;
		bool kickImmediately;
	};

	//for driving to ball for taking a kick
	class FindTargetState : public State {

	public:
		FindTargetState(TeamController* ai) : State(ai) {}
		void onEnter(Robot* robot, Parameters parameters);
		void step(float dt, Vision::Results* visionResults, Robot* robot, float totalDuration, float stateDuration, float combinedDuration);
	private:
		std::string lastState;
		std::string nextState;
		std::string targetType;
		std::string kickType;
		bool kickImmediately;
		int validCount;
		bool areaLocked;
		Part lockedArea;
	};

	//for driving around the opponent with back to opponent
	class BackAroundOpponentState : public State {

	public:
		BackAroundOpponentState(TeamController* ai) : State(ai) {}
		void onEnter(Robot* robot, Parameters parameters);
		void step(float dt, Vision::Results* visionResults, Robot* robot, float totalDuration, float stateDuration, float combinedDuration);
	private:
		std::string lastState;
		std::string nextState;
		int opponentSeenCounter;
		int opponentLostCounter;
		Object* lastEnemyRobot;
		bool rotateClockwise;
	};

	//Press an enemy robot
	class PressOpponentState : public State {

	public:
		PressOpponentState(TeamController* ai) : State(ai), kP(3.0f), kI(1.0f), kD(0.0035f), pid(kP, kI, kD, 0.016f), pidUpdateCounter(0) {}
		void onEnter(Robot* robot, Parameters parameters);
		void step(float dt, Vision::Results* visionResults, Robot* robot, float totalDuration, float stateDuration, float combinedDuration);
	private:
		float maxSideSpeed;
		int opponentLostCounter;
		Object* lastEnemyRobot;

		PID pid;
		float kP;
		float kI;
		float kD;
		char pidUpdateCounter;
	};

	//For maneuvering to optimal positions for situations, don't know if will have time to implement properly
	class ManeuverState : public State {

	public:
		ManeuverState(TeamController* ai) : State(ai) {}
		void onEnter(Robot* robot, Parameters parameters);
		void step(float dt, Vision::Results* visionResults, Robot* robot, float totalDuration, float stateDuration, float combinedDuration);
	};

	TeamController(Robot* robot, AbstractCommunication* com, Client* client);
	~TeamController();

	void reset() override;
	void handleRefereeCommand(const Command& cmd);
	float getChipKickDistance(float targetDistance);
	std::string getSituationName(GameSituation situation);
	std::string getTeamPossessionName(TeamInPossession team);
	std::string getJSON();

private:
	void setupStates();
	GameSituation currentSituation;
	TeamInPossession whoHasBall;
	bool passNeeded;
	bool isCaptain;
	int friendlyGoalCounter;
	int enemyGoalCounter;
	int passStrength;
	int directKickStrength;
	float chipKickAdjust;
};

#endif // TEAMCONTROLLER_H