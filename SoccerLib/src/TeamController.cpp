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
	passNeeded = true;
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
	states["wait-for-referee"] = new WaitForRefereeState(this);
	states["manual-control"] = new ManualControlState(this);
	states["drive-to"] = new DriveToState(this);
	states["turn-by"] = new TurnByState(this);
	states["wait-for-kick"] = new WaitForKickState(this);
	states["defend-goal"] = new DefendGoalState(this);
	states["intercept-ball"] = new InterceptBallState(this);
	states["take-kickoff"] = new TakeKickoffState(this);
	states["take-freekick-direct"] = new TakeFreeKickDirectState(this);
	states["take-freekick-indirect"] = new TakeFreeKickIndirectState(this);
	states["take-goalkick"] = new TakeGoalkickState(this);
	states["take-throwin"] = new TakeThrowInState(this);
	states["take-cornerkick"] = new TakeCornerKickState(this);
	states["take-penalty"] = new TakePenaltyState(this);
	states["find-ball"] = new FindBallState(this);
	states["find-ball-goalkeeper"] = new FindBallGoalkeeperState(this);
	states["fetch-ball-front"] = new FetchBallFrontState(this);
	states["fetch-ball-rear"] = new FetchBallRearState(this);
	states["drive-to-own-goal"] = new DriveToOwnGoalState(this);
	states["aim-kick"] = new AimKickState(this);
	states["pass-ball"] = new PassBallState(this);
	states["get-pass"] = new GetPassState(this);
}

void TeamController::WaitForRefereeState::onEnter(Robot* robot, Parameters parameters) {
	//Nullify previous referee state
	ai->whoHasBall = TeamInPossession::NOONE;
	ai->currentSituation = GameSituation::UNKNOWN;
}

void TeamController::WaitForRefereeState::step(float dt, Vision::Results* visionResults, Robot* robot, float totalDuration, float stateDuration, float combinedDuration) {
	//this state will be entered after the referee sends stop and waits for the next command from the referee to choose next state
}

void TeamController::WaitForKickState::onEnter(Robot* robot, Parameters parameters) {
	//reset starting ball position
	startingBallPos.x = -1000.0f;
	startingBallPos.y = -1000.0f;
}

void TeamController::WaitForKickState::step(float dt, Vision::Results* visionResults, Robot* robot, float totalDuration, float stateDuration, float combinedDuration) {
	Object* ball = visionResults->getClosestBall();

	// configuration parameters
	float kickDetectionDeltaPosition = 0.1f;
	float kickDetectionMovingSpeed = 0.6f;

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
			if (abs(currentBallPos.x - startingBallPos.x) > kickDetectionDeltaPosition || abs(currentBallPos.y - startingBallPos.y) > kickDetectionDeltaPosition /*|| ball->relativeMovement.speed > kickDetectionMovingSpeed*/) {
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
		//TODO temporary substitute
		ai->setState("manual-control");
		return;
	}

	//probably should add function to vision to get fastest moving ball instead, but in 2v2 this should work, as there is only one ball
	Object* ball = visionResults->getClosestBall();

	// configuration parameters
	float maximumFetchDistance = 0.15f;
	float minimumMovingDeltaY = 0.05f;
	float goalKeepDistance = 0.5f;
	float forwardSpeedMultiplier = 10.0f;
	float sidewaysSpeedMultiplier = 10.0f;

	//if can't see ball
	if (ball == NULL) {
		//if ball has not been seen before, start scanning for ball
		if (!ballWasSeen)
		{
			ai->setState("find-ball-goalkeeper");
			return;
		} //if ball was seen, do something like track robot closest to centre or just keep current position or something whatever
		else {
			//TODO write something here
		}
	} //ball found, check if it isn't in a goal
	else if (!visionResults->isBallInGoal(ball)) {
		if (!ballWasSeen) ballWasSeen = true;

		bool shouldIntercept = false;
		float forwardSpeed = 0.0f, sideWaysSpeed = 0.0f;
		
		//check if ball is close enough to fetch
		if (ball->distance < maximumFetchDistance) {
			if (ball->behind) {
				ai->setState("fetch-ball-rear");
				return;
			}
			else {
				ai->setState("fetch-ball-front");
				return;
			}
		}
		
		//check if ball is moving fast enough toward robot
		if (ball->relativeMovement.dY > minimumMovingDeltaY) {
			shouldIntercept = true;
		}

		if (shouldIntercept) {
			ai->setState("intercept-ball");
			return;
		}
		else {
			//calculate parameters to keep robot in front of goal, current calculation doesn't probably really work
			forwardSpeed = (goalKeepDistance - defendedGoal->distance) * forwardSpeedMultiplier;
			sideWaysSpeed = defendedGoal->distanceX * sidewaysSpeedMultiplier;
		}
		robot->setTargetDir(forwardSpeed, sideWaysSpeed);
		robot->lookAt(ball);
		
	}
}

void TeamController::InterceptBallState::onEnter(Robot* robot, Parameters parameters) {
	//TODO fill this out
}

void TeamController::InterceptBallState::step(float dt, Vision::Results* visionResults, Robot* robot, float totalDuration, float stateDuration, float combinedDuration) {
	//TODO fill this out
}

void TeamController::TakeKickoffState::onEnter(Robot* robot, Parameters parameters) {
	//TODO fill this out
}

void TeamController::TakeKickoffState::step(float dt, Vision::Results* visionResults, Robot* robot, float totalDuration, float stateDuration, float combinedDuration) {
	//TODO fill this out
}

void TeamController::TakeFreeKickDirectState::onEnter(Robot* robot, Parameters parameters) {
	//TODO fill this out
}

void TeamController::TakeFreeKickDirectState::step(float dt, Vision::Results* visionResults, Robot* robot, float totalDuration, float stateDuration, float combinedDuration) {
	//TODO fill this out
}

void TeamController::TakeFreeKickIndirectState::onEnter(Robot* robot, Parameters parameters) {
	//TODO fill this out
}

void TeamController::TakeFreeKickIndirectState::step(float dt, Vision::Results* visionResults, Robot* robot, float totalDuration, float stateDuration, float combinedDuration) {
	//TODO fill this out
}

void TeamController::TakeGoalkickState::onEnter(Robot* robot, Parameters parameters) {
	//TODO fill this out
}

void TeamController::TakeGoalkickState::step(float dt, Vision::Results* visionResults, Robot* robot, float totalDuration, float stateDuration, float combinedDuration) {
	//TODO fill this out
}

void TeamController::TakeThrowInState::onEnter(Robot* robot, Parameters parameters) {
	//TODO fill this out
}

void TeamController::TakeThrowInState::step(float dt, Vision::Results* visionResults, Robot* robot, float totalDuration, float stateDuration, float combinedDuration) {
	//TODO fill this out
}

void TeamController::TakeCornerKickState::onEnter(Robot* robot, Parameters parameters) {
	//TODO fill this out
}

void TeamController::TakeCornerKickState::step(float dt, Vision::Results* visionResults, Robot* robot, float totalDuration, float stateDuration, float combinedDuration) {
	//TODO fill this out
}

void TeamController::TakePenaltyState::onEnter(Robot* robot, Parameters parameters) {
	//TODO fill this out
}

void TeamController::TakePenaltyState::step(float dt, Vision::Results* visionResults, Robot* robot, float totalDuration, float stateDuration, float combinedDuration) {
	//TODO fill this out
}

void TeamController::FindBallState::onEnter(Robot* robot, Parameters parameters) {
	//TODO fill this out
}

void TeamController::FindBallState::step(float dt, Vision::Results* visionResults, Robot* robot, float totalDuration, float stateDuration, float combinedDuration) {
	//TODO fill this out
}

void TeamController::FindBallGoalkeeperState::onEnter(Robot* robot, Parameters parameters) {
	//TODO fill this out
}

void TeamController::FindBallGoalkeeperState::step(float dt, Vision::Results* visionResults, Robot* robot, float totalDuration, float stateDuration, float combinedDuration) {
	//TODO fill this out
}

void TeamController::FetchBallFrontState::onEnter(Robot* robot, Parameters parameters) {
	//TODO fill this out
}

void TeamController::FetchBallFrontState::step(float dt, Vision::Results* visionResults, Robot* robot, float totalDuration, float stateDuration, float combinedDuration) {
	//TODO fill this out
}

void TeamController::FetchBallRearState::onEnter(Robot* robot, Parameters parameters) {
	//TODO fill this out
}

void TeamController::FetchBallRearState::step(float dt, Vision::Results* visionResults, Robot* robot, float totalDuration, float stateDuration, float combinedDuration) {
	//TODO fill this out
}

void TeamController::DriveToOwnGoalState::onEnter(Robot* robot, Parameters parameters) {
	//TODO fill this out
}

void TeamController::DriveToOwnGoalState::step(float dt, Vision::Results* visionResults, Robot* robot, float totalDuration, float stateDuration, float combinedDuration) {
	//TODO fill this out
}

void TeamController::AimKickState::onEnter(Robot* robot, Parameters parameters) {
	//TODO fill this out
}

void TeamController::AimKickState::step(float dt, Vision::Results* visionResults, Robot* robot, float totalDuration, float stateDuration, float combinedDuration) {
	//TODO fill this out
}

void TeamController::PassBallState::onEnter(Robot* robot, Parameters parameters) {
	//TODO fill this out
}

void TeamController::PassBallState::step(float dt, Vision::Results* visionResults, Robot* robot, float totalDuration, float stateDuration, float combinedDuration) {
	//TODO fill this out
}

void TeamController::GetPassState::onEnter(Robot* robot, Parameters parameters) {
	//TODO fill this out
}

void TeamController::GetPassState::step(float dt, Vision::Results* visionResults, Robot* robot, float totalDuration, float stateDuration, float combinedDuration) {
	//TODO fill this out
}