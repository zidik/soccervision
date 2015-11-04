#ifndef TEAMCONTROLLER_H
#define TEAMCONTROLLER_H

#include "TestController.h"

class TeamController : public TestController {

public:
	enum GameSituation { UNKNOWN = -1, KICKOFF = 0, INDIRECTFREEKICK = 1, DIRECTFREEKICK = 2, GOALKICK = 3, THROWIN = 4, CORNERKICK = 4, PENALTY = 6 };
	enum TeamInPossession { ENEMY = -1, NOONE = 0, FRIENDLY = 1 };

	//This state detects if ball has been kicked starting with a stationary robot
	class WaitForKickState : public State {

	public:
		WaitForKickState(TeamController* ai) : State(ai), startingBallPos(-1000.0f, -1000.0f) {}
		void onEnter(Robot* robot, Parameters parameters);
		void step(float dt, Vision::Results* visionResults, Robot* robot, float totalDuration, float stateDuration, float combinedDuration);

	private:
		Math::Vector startingBallPos;
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

	TeamController(Robot* robot, AbstractCommunication* com);
	~TeamController();

	void reset() override;

private:
	void setupStates();
	GameSituation currentSituation;
	TeamInPossession whoHasBall;
};

#endif // TEAMCONTROLLER_H