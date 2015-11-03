#include "TeamController.h"

#include "Robot.h"
#include "Dribbler.h"
#include "Coilgun.h"
#include "Command.h"

TeamController::TeamController(Robot* robot, AbstractCommunication* com) : TestController(robot, com) {
	setupStates();

	speedMultiplier = 1.0f;
	whoHasBall = TeamInPossession::NOONE;
	currentSituation = GameSituation::UNKNOWN;
};

TeamController::~TeamController() {

}

void TeamController::setupStates() {
	states["manual-control"] = new ManualControlState(this);
	states["drive-to"] = new DriveToState(this);
	states["turn-by"] = new TurnByState(this);
	states["wait-for-kick"] = new WaitForKickState(this);
}

void TeamController::WaitForKickState::onEnter(Robot* robot, Parameters parameters) {
	
}

void TeamController::WaitForKickState::step(float dt, Vision::Results* visionResults, Robot* robot, float totalDuration, float stateDuration, float combinedDuration) {
	//TO-DO write logic here
	Object* ball = visionResults->getClosestBall();

	//write down initial ball location if not done already, relative coordinates
	if (startingBallPos.x < -999.0f){
		if (ball != NULL) {
			startingBallPos.x = ball->distanceX;
			startingBallPos.y = ball->distanceY;
		}
	}
	else {
		if (ball != NULL) {	
			Math::Vector currentBallPos = Math::Vector(ball->distanceX, ball->distanceY);

			//check if ball has drifted significantly away from starting position or is moving with high enough speed
			if (abs(currentBallPos.x - startingBallPos.x) < 0.2f || abs(currentBallPos.y - startingBallPos.y) < 0.2f || ball->relativeMovement.speed > 0.6f) {
				//kick detected
				ai->setState("manual-control");
			}
		}
	}
}