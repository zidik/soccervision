#ifndef TEAMCONTROLLER_H
#define TEAMCONTROLLER_H

#include "TestController.h"

class TeamController : public TestController {

public:
	enum GameSituation { UNKNOWN = -1, KICKOFF = 0, INDIRECTFREEKICK = 1, DIRECTFREEKICK = 2, GOALKICK = 3, THROWIN = 4, CORNERKICK = 4, PENALTY = 6 };
	enum TeamInPossession { ENEMY = -1, NOONE = 0, FRIENDLY = 1 };

	class WaitForKickState : public State {

	public:
		WaitForKickState(TestController* ai) : State(ai), ballIsInCentre(false), ballNearEdge(false), startingBallPos(0.0f, 0.0f) {}
		void onEnter(Robot* robot, Parameters parameters);
		void step(float dt, Vision::Results* visionResults, Robot* robot, float totalDuration, float stateDuration, float combinedDuration);

	private:
		bool ballIsInCentre;
		bool ballNearEdge;
		Math::Vector startingBallPos;
	};

	TeamController(Robot* robot, AbstractCommunication* com);
	~TeamController();

private:
	void setupStates();
	GameSituation currentSituation;
	TeamInPossession whoHasBall;
};

#endif // TEAMCONTROLLER_H