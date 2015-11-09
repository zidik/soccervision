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

void TeamController::reset() {
	std::cout << "! Reset team-controller" << std::endl;

	com->send("reset");
	targetSide = Side::YELLOW; // will be switched to blue in handleToggleSideCommand()
	defendSide = Side::BLUE;
	totalDuration = 0.0f;
	currentStateDuration = 0.0f;
	currentState = NULL;
	currentStateName = "";

	setState("manual-control");
	handleToggleSideCommand();
}

void TeamController::setupStates() {
	states["manual-control"] = new ManualControlState(this);
	states["drive-to"] = new DriveToState(this);
	states["turn-by"] = new TurnByState(this);
	states["wait-for-kick"] = new WaitForKickState(this);
	states["defend-goal"] = new DefendGoalState(this);
}

void TeamController::WaitForKickState::onEnter(Robot* robot, Parameters parameters) {
	//reset starting ball position
	startingBallPos.x = -1000.0f;
	startingBallPos.y = -1000.0f;
}

void TeamController::WaitForKickState::step(float dt, Vision::Results* visionResults, Robot* robot, float totalDuration, float stateDuration, float combinedDuration) {
	Object* ball = visionResults->getClosestBall();

	if (ball != NULL) {
		//write down initial ball location if not done already, relative coordinates
		if (startingBallPos.x < -999.0f){
			startingBallPos.x = ball->distanceX;
			startingBallPos.y = ball->distanceY;

			//std::cout << "Found ball @ position- x:" << ball->distanceX << " y: " << ball->distanceY << std::endl;
		}
		else {
			Math::Vector currentBallPos = Math::Vector(ball->distanceX, ball->distanceY);

			//check if ball has drifted significantly away from starting position or is moving with high enough speed (not using speed currently, speed calculation has too many errors it seems)
			if (abs(currentBallPos.x - startingBallPos.x) > 0.15f || abs(currentBallPos.y - startingBallPos.y) > 0.15f /*|| ball->relativeMovement.speed > 0.6f*/) {
				//kick detected

				//TO-DO write here what states to go to based on game situation, currently
				ai->setState("manual-control");
				return;
			}
		}
	}	
}

void TeamController::DefendGoalState::onEnter(Robot* robot, Parameters parameters) {
	ballWasSeen = false;
}

void TeamController::DefendGoalState::step(float dt, Vision::Results* visionResults, Robot* robot, float totalDuration, float stateDuration, float combinedDuration) {
	//TODO write goal defending logic here.

	Object* defendedGoal = visionResults->getLargestGoal(ai->getDefendSide(), Dir::REAR);

	//if goal is not visible in back camera, switch to driving in front of goal state.
	if (defendedGoal == NULL) {
		//temporary substitute
		ai->setState("manual-control");
		return;
	}

	Object* ball = visionResults->getClosestBall();

	// configuration parameters
	float maximumFetchDistance = 0.15f;
	float minimumMovingDeltaY = 0.1f;
	float goalKeepDistance = 0.5f;
	float forwardSpeedMultiplier = 10.0f;
	float sidewaysSpeedMultiplier = 10.0f;

	//if can't see ball
	if (ball == NULL) {
		//if ball has not been seen before, start scanning for ball
		if (!ballWasSeen)
		{
			//temporary substitute
			ai->setState("manual-control");
			return;
		} //if ball was seen, do something like track robot closest to centre or something
		else {
			//temporary substitute
			ai->setState("manual-control");
			return;
		}
	} //ball found
	else {
		if (!ballWasSeen) ballWasSeen = true;

		bool shouldIntercept = false;
		float forwardSpeed = 0.0f, sideWaysSpeed = 0.0f;
		
		//check if ball is close enough to fetch
		if (ball->distance < maximumFetchDistance) {
			//temporary substitute
			ai->setState("manual-control");
			return;
		}
		
		//check if ball is moving fast enough toward robot
		if (ball->relativeMovement.dY > minimumMovingDeltaY) {
			shouldIntercept = true;
		}

		if (shouldIntercept) {
			//calculate optimal movement parameters
			forwardSpeed = (goalKeepDistance - defendedGoal->distance) * forwardSpeedMultiplier;
			sideWaysSpeed = ball->relativeMovement.dX * sidewaysSpeedMultiplier;
		}
		else {
			//centre robot in front of goal, this one is not a very good solution, but temporary
			forwardSpeed = (goalKeepDistance - defendedGoal->distance) * forwardSpeedMultiplier;
			sideWaysSpeed = defendedGoal->distanceX * sidewaysSpeedMultiplier;
		}
		robot->setTargetDir(forwardSpeed, sideWaysSpeed);
		robot->lookAt(ball);
		
	}
}