#ifndef TEAMCONTROLLER_H
#define TEAMCONTROLLER_H

#include "TestController.h"

class TeamController : public TestController {

public:
	enum GameSituation { UNKNOWN = -1, KICKOFF = 0, INDIRECTFREEKICK = 1, DIRECTFREEKICK = 2, GOALKICK = 3, THROWIN = 4, CORNERKICK = 4, PENALTY = 6, ENDHALF = 7 };
	enum TeamInPossession { NOONE = -1, ENEMY = 0, FRIENDLY = 1 };

	class State : public BaseAI::State {

	public:
		State(TeamController* ai) : BaseAI::State(ai), ai(ai) {}

	protected:
		TeamController* ai;

	};

	//This state keeps the robot waiting for a referee command about what situation is next
	class WaitForRefereeState : public State {

	public:
		WaitForRefereeState(TeamController* ai) : State(ai) {}
		void onEnter(Robot* robot, Parameters parameters);
		void step(float dt, Vision::Results* visionResults, Robot* robot, float totalDuration, float stateDuration, float combinedDuration);
	};

	//This state detects if ball has been kicked starting with a stationary robot
	class WaitForKickState : public State {

	public:
		WaitForKickState(TeamController* ai) : State(ai), startingBallPos(-1000.0f, -1000.0f) {}
		void onEnter(Robot* robot, Parameters parameters);
		void step(float dt, Vision::Results* visionResults, Robot* robot, float totalDuration, float stateDuration, float combinedDuration);

	private:
		Math::Vector startingBallPos;
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
		DefendGoalState(TeamController* ai) : State(ai), ballWasSeen(false) {}
		void onEnter(Robot* robot, Parameters parameters);
		void step(float dt, Vision::Results* visionResults, Robot* robot, float totalDuration, float stateDuration, float combinedDuration);

	private:
		bool ballWasSeen;
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
		FetchBallFrontState(TeamController* ai) : State(ai) {}
		void onEnter(Robot* robot, Parameters parameters);
		void step(float dt, Vision::Results* visionResults, Robot* robot, float totalDuration, float stateDuration, float combinedDuration);
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

	private:
		float goalSearchDir;
		bool droveTowardEnemyGoal;
		bool droveTowardFriendlyGoal;
		bool searchedEnemyGoal;
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

private:
	void setupStates();
	GameSituation currentSituation;
	TeamInPossession whoHasBall;
	bool passNeeded;
	bool isCaptain;
};

#endif // TEAMCONTROLLER_H