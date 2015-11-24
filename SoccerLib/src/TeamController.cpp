#include "TeamController.h"

#include "Robot.h"
#include "Dribbler.h"
#include "Coilgun.h"
#include "Command.h"

TeamController::TeamController(Robot* robot, AbstractCommunication* com, Client* client) : TestController(robot, com, client) {
	setupStates();

	speedMultiplier = 1.0f;
	whoHasBall = TeamInPossession::NOONE;
	currentSituation = GameSituation::UNKNOWN;
	passNeeded = true;
	isCaptain = false;
	friendlyGoalCounter = 0;
	enemyGoalCounter = 0;
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
	friendlyGoalCounter = 0;
	enemyGoalCounter = 0;

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
	states["maneuver"] = new ManeuverState(this);
}

void TeamController::handleRefereeCommand(const Command& cmd)
{
	if (cmd.parameters[0][1] == fieldID) {
		if (cmd.parameters[0][2] == robotID || cmd.parameters[0][2] == 'X') {
			if (cmd.parameters[0][3] == 'A' || cmd.parameters[0][3] == 'B') {
				bool commandForOurTeam = cmd.parameters[0][3] == teamID;
				std::string command = cmd.parameters[0].substr(4);

				if (commandForOurTeam) {
					whoHasBall = TeamInPossession::FRIENDLY;
				}
				else {
					whoHasBall = TeamInPossession::ENEMY;
				}

				if (command == "KICKOFF-"){ currentSituation = GameSituation::KICKOFF; }
				else if (command == "IFREEK--"){ currentSituation = GameSituation::INDIRECTFREEKICK; }
				else if (command == "DFREEK--"){ currentSituation = GameSituation::DIRECTFREEKICK; }
				else if (command == "GOALK---"){ currentSituation = GameSituation::GOALKICK; }
				else if (command == "THROWIN-"){ currentSituation = GameSituation::THROWIN; }
				else if (command == "CORNERK-"){ currentSituation = GameSituation::CORNERKICK; }
				else if (command == "PENALTY-"){ currentSituation = GameSituation::PENALTY; }
				else if (command == "GOAL----"){
					if (commandForOurTeam) friendlyGoalCounter++;
					else enemyGoalCounter++;
				}
			} else {
				std::string command = cmd.parameters[0].substr(3);

				if (command == "START----") { 
					std::cout << "Teamcontroller start" << std::endl;
					if (whoHasBall == TeamInPossession::FRIENDLY) {
						switch (currentSituation){
						case GameSituation::KICKOFF:
							setState("take-kickoff");
							return;
						case GameSituation::INDIRECTFREEKICK:
							setState("take-freekick-indirect");
							return;
						case GameSituation::DIRECTFREEKICK:
							setState("take-freekick-direct");
							return;
						case GameSituation::GOALKICK:
							setState("take-goalkick");
							return;
						case GameSituation::THROWIN:
							setState("take-throwin");
							return;
						case GameSituation::CORNERKICK:
							setState("take-cornerkick");
							return;
						case GameSituation::PENALTY:
							setState("take-penalty");
							return;
						case GameSituation::PLACEDBALL:
							setState("find-ball");
							return;
						}
					}
					else {
						setState("wait-for-kick");
						return;
					}
				}
				else if (command == "STOP-----") {
					setState("manual-control");
					return;
				}
				else if (command == "PLACEDBAL") {
					currentSituation = GameSituation::PLACEDBALL;
					whoHasBall = TeamInPossession::FRIENDLY;
				}
				else if (command == "ENDHALF--") {
					std::cout << "- half ended - ROADKILL " << friendlyGoalCounter << ":" << enemyGoalCounter << " OTHERS" << std::endl;
					setState("manual-control");
					return;
				}
			}			
		}
	}
}

void TeamController::WaitForRefereeState::onEnter(Robot* robot, Parameters parameters) {
	//Nullify previous referee state
	ai->whoHasBall = TeamInPossession::NOONE;
	ai->currentSituation = GameSituation::UNKNOWN;
}

void TeamController::WaitForRefereeState::step(float dt, Vision::Results* visionResults, Robot* robot, float totalDuration, float stateDuration, float combinedDuration) {
	//this state will be entered after the referee sends stop and waits for the next command from the referee to choose next state

	//check if referee has sent next situation
	if (ai->currentSituation != GameSituation::UNKNOWN && ai->isCaptain) {
		Parameters parameters;
		switch (ai->currentSituation) {
		case GameSituation::KICKOFF:
			if (ai->whoHasBall == TeamInPossession::FRIENDLY) {
				parameters["next-state"] = "take-kickoff";
				parameters["team-in-possession"] = "1";
			}
			else {

			}
			break;
		}
	}
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
				//if ()
				switch (ai->currentSituation) {
				case GameSituation::KICKOFF:
					ai->setState("");
					//TODO send to teammate to defend goal
					return;
				}

			}
		}
	}	
}

void TeamController::DefendGoalState::onEnter(Robot* robot, Parameters parameters) {
	ballWasSeen = false;
}

void TeamController::DefendGoalState::step(float dt, Vision::Results* visionResults, Robot* robot, float totalDuration, float stateDuration, float combinedDuration) {
	//TODO write goal defending logic here.

	if (robot->dribbler->gotBall()) {
		Parameters parameters;
		parameters["next-state"] = "defend-goal";
		if (ai->passNeeded) {
			ai->setState("pass-ball", parameters);
		}
		else {
			ai->setState("aim-kick", parameters);
		}
		return;
	}

	Object* defendedGoal = visionResults->getLargestGoal(ai->getDefendSide(), Dir::REAR);

	//if goal is not visible in back camera, switch to driving in front of goal state.
	if (defendedGoal == NULL) {
		ai->setState("drive-to-own-goal");
		return;
	}

	//probably should add function to vision to get fastest moving ball instead, but in 2v2 this should work, as there is only one ball
	Object* ball = visionResults->getClosestBall();

	// configuration parameters
	float maximumFetchDistance = 0.15f;
	float minimumMovingDeltaY = -0.05f;
	float goalKeepDistance = 0.5f;
	float forwardSpeedMultiplier = 1.0f;
	float sidewaysSpeedMultiplier = 1.0f;

	//if can't see ball
	if (ball == NULL) {
		//scan for ball
		robot->stop();
		ai->setState("find-ball-goalkeeper");
		return;
		
		//using simpler solution for now, where robot will search for ball whenever it doesn't see it
		/*
		//if ball has not been seen before, start scanning for ball
		if (!ballWasSeen)
		{
			ai->setState("find-ball-goalkeeper");
			return;
		} //if ball was seen, do something like track robot closest to centre or just keep current position or something whatever
		else {
			//TODO write something here
		}
		*/
	} //ball found, check if it isn't in a goal
	else if (!visionResults->isBallInGoal(ball)) {
		if (!ballWasSeen) ballWasSeen = true;

		bool shouldIntercept = false;
		float forwardSpeed = 0.0f, sideWaysSpeed = 0.0f;
		
		//check if ball is close enough to fetch
		if (ball->distance < maximumFetchDistance) {
			robot->stop();
			if (ball->behind) {
				ai->setState("fetch-ball-rear");
			}
			else {
				ai->setState("fetch-ball-front");
			}
			return;
		}
		
		//check if ball is moving fast enough toward robot
		if (ball->relativeMovement.dY < minimumMovingDeltaY) {
			shouldIntercept = true;
		}

		if (shouldIntercept) {
			robot->stop();
			ai->setState("intercept-ball");
			return;
		}
		else {
			//calculate parameters to keep robot in front of goal, this version untested and likely doesn't work
			forwardSpeed = (goalKeepDistance - defendedGoal->distance) * forwardSpeedMultiplier;
			sideWaysSpeed = defendedGoal->distanceX * -sidewaysSpeedMultiplier;
		}
		robot->setTargetDir(forwardSpeed, sideWaysSpeed);
		robot->lookAt(ball);
		
	}
}

void TeamController::InterceptBallState::onEnter(Robot* robot, Parameters parameters) {
	//TODO fill this out
}

void TeamController::InterceptBallState::step(float dt, Vision::Results* visionResults, Robot* robot, float totalDuration, float stateDuration, float combinedDuration) {
	if (robot->dribbler->gotBall()) {
		Parameters parameters;
		parameters["next-state"] = "defend-goal";
		if (ai->passNeeded) {
			ai->setState("pass-ball", parameters);
		}
		else {
			ai->setState("aim-kick", parameters);
		}
		return;
	}

	Object* ball = visionResults->getClosestBall();

	//configuration parameters
	float ballFetchMinDistance = 0.3f;
	float sidewaysSpeedMultiplier = 3.0f;

	//if can't see ball
	if (ball == NULL) {
		ai->setState("defend-goal");
		return;
	} //ball found, check if it isn't in a goal
	else if (!visionResults->isBallInGoal(ball)) {
		//if ball is close, fetch it.
		if (ball->distance < ballFetchMinDistance) {
			Parameters parameters;
			parameters["next-state"] = "defend-goal";
			if (!ball->behind) ai->setState("fetch-ball-front", parameters);
			else  ai->setState("fetch-ball-behind", parameters);
			return;
		}
		else {
			float forwardSpeed = 0.0f;
			float sidewaysSpeed = 0.0f;

			sidewaysSpeed = ball->distanceX * sidewaysSpeedMultiplier;

			robot->setTargetDir(forwardSpeed, sidewaysSpeed);
		}
	}

}

void TeamController::TakeKickoffState::onEnter(Robot* robot, Parameters parameters) {
	ai->passNeeded = true;
	//ai->client->send("run-get-pass");
}

void TeamController::TakeKickoffState::step(float dt, Vision::Results* visionResults, Robot* robot, float totalDuration, float stateDuration, float combinedDuration) {
	Object* ball = visionResults->getClosestBall(Dir::FRONT);
	Object* teamMate = visionResults->getLargestRobot(ai->teamColor, Dir::FRONT);

	if (robot->dribbler->gotBall()) {
		Parameters parameters;
		parameters["next-state"] = "manual-control";
		parameters["last-state"] = "take-kickoff";
		parameters["kick-type"] = "pass";
		parameters["target-type"] = "team-robot";
		ai->setState("aim-kick", parameters);
		//robot->dribbler->stop();
		//robot->stop();
		return;
	}

	if (ball == NULL) {
		Parameters parameters;
		parameters["next-state"] = "take-kickoff";
		ai->setState("find-ball", parameters);
	} else {
		float forwardSpeed = 0.0f;
		float sidewaysSpeed = 0.0f;

		//configuration parameters
		float forwardSpeedMult = 1.5f;
		float sidewaysSpeedMult = 0.3f;
		float robotSearchDir = -1.0f;
		float minForwardSpeed = 0.1f;
		float ballRotateDistance = 0.2f;
		float ballDistanceError = 0.05f;
		float teamMateSearchSpeed = 0.4f;
		float robotAngleError = Math::PI / 30.0f;

		if (teamMate != NULL) {
			if (abs(teamMate->angle) < robotAngleError) {
				//move toward ball
				forwardSpeed = minForwardSpeed + ball->distance * forwardSpeedMult;
				sidewaysSpeed = ball->distanceX * sidewaysSpeedMult;
				robot->dribbler->start();
			}
			else {
				//turn toward teammate
				sidewaysSpeed = teamMate->angle * -sidewaysSpeedMult;
				forwardSpeed = (ball->distance - ballRotateDistance) * forwardSpeedMult;
			}
		}
		else {
			if (abs(ball->distance - ballRotateDistance) < ballDistanceError) {
				//search for teammate
				forwardSpeed = (ball->distance - ballRotateDistance) * forwardSpeedMult;
				sidewaysSpeed = teamMateSearchSpeed * robotSearchDir;
			}
			else {
				//move ball to correct distance
				forwardSpeed = (ball->distance - ballRotateDistance) * forwardSpeedMult;
				sidewaysSpeed = ball->distanceX * sidewaysSpeedMult;
			}
		}
		
		robot->setTargetDir(forwardSpeed, sidewaysSpeed);
		robot->lookAt(ball);

	}
}

void TeamController::TakeFreeKickDirectState::onEnter(Robot* robot, Parameters parameters) {
	//TODO fill this out
}

void TeamController::TakeFreeKickDirectState::step(float dt, Vision::Results* visionResults, Robot* robot, float totalDuration, float stateDuration, float combinedDuration) {
	Object* ball = visionResults->getClosestBall(Dir::FRONT);
	Object* goal = visionResults->getLargestGoal(ai->targetSide, Dir::FRONT);

	if (robot->dribbler->gotBall()) {
		Parameters parameters;
		parameters["next-state"] = "manual-control";
		parameters["last-state"] = "take-freekick-direct";
		parameters["kick-type"] = "chip";
		parameters["target-type"] = "enemy-goal";
		ai->setState("aim-kick", parameters);
		//robot->dribbler->stop();
		//robot->stop();
		return;
	}

	if (ball == NULL) {
		Parameters parameters;
		parameters["next-state"] = "take-freekick-direct";
		ai->setState("find-ball", parameters);
	}
	else {
		float forwardSpeed = 0.0f;
		float sidewaysSpeed = 0.0f;

		//configuration parameters
		float forwardSpeedMult = 1.5f;
		float sidewaysSpeedMult = 0.3f;
		float robotSearchDir = -1.0f;
		float minForwardSpeed = 0.1f;
		float ballRotateDistance = 0.2f;
		float ballDistanceError = 0.05f;
		float teamMateSearchSpeed = 0.4f;
		float goalAngleError = Math::PI / 30.0f;

		if (goal != NULL) {
			if (abs(goal->angle) < goalAngleError) {
				//move toward ball
				forwardSpeed = minForwardSpeed + ball->distance * forwardSpeedMult;
				sidewaysSpeed = ball->distanceX * sidewaysSpeedMult;
				robot->dribbler->start();
			}
			else {
				//turn toward teammate
				sidewaysSpeed = goal->angle * -sidewaysSpeedMult;
				forwardSpeed = (ball->distance - ballRotateDistance) * forwardSpeedMult;
			}
		}
		else {
			if (abs(ball->distance - ballRotateDistance) < ballDistanceError) {
				//search for goal
				forwardSpeed = (ball->distance - ballRotateDistance) * forwardSpeedMult;
				sidewaysSpeed = teamMateSearchSpeed * robotSearchDir;
			}
			else {
				//move ball to correct distance
				forwardSpeed = (ball->distance - ballRotateDistance) * forwardSpeedMult;
				sidewaysSpeed = ball->distanceX * sidewaysSpeedMult;
			}
		}

		robot->setTargetDir(forwardSpeed, sidewaysSpeed);
		robot->lookAt(ball);

	}
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
	Object* ball = visionResults->getClosestBall(Dir::FRONT);
	Object* goal = visionResults->getLargestGoal(ai->targetSide, Dir::FRONT);

	if (robot->dribbler->gotBall()) {
		Parameters parameters;
		parameters["next-state"] = "manual-control";
		parameters["last-state"] = "take-penalty";
		parameters["kick-type"] = "direct";
		parameters["target-type"] = "enemy-goal";
		ai->setState("aim-kick", parameters);
		//robot->dribbler->stop();
		//robot->stop();
		return;
	}

	if (ball == NULL) {
		Parameters parameters;
		parameters["next-state"] = "take-penalty";
		ai->setState("find-ball", parameters);
	}
	else {
		float forwardSpeed = 0.0f;
		float sidewaysSpeed = 0.0f;

		//configuration parameters
		float forwardSpeedMult = 1.5f;
		float sidewaysSpeedMult = 0.3f;
		float robotSearchDir = -1.0f;
		float minForwardSpeed = 0.1f;
		float ballRotateDistance = 0.2f;
		float ballDistanceError = 0.05f;
		float teamMateSearchSpeed = 0.4f;
		float goalAngleError = Math::PI / 30.0f;

		if (goal != NULL) {
			if (abs(goal->angle) < goalAngleError) {
				//move toward ball
				forwardSpeed = minForwardSpeed + ball->distance * forwardSpeedMult;
				sidewaysSpeed = ball->distanceX * sidewaysSpeedMult;
				robot->dribbler->start();
			}
			else {
				//turn toward goal
				sidewaysSpeed = goal->angle * -sidewaysSpeedMult;
				forwardSpeed = (ball->distance - ballRotateDistance) * forwardSpeedMult;
			}
		}
		else {
			if (abs(ball->distance - ballRotateDistance) < ballDistanceError) {
				//search for goal
				forwardSpeed = (ball->distance - ballRotateDistance) * forwardSpeedMult;
				sidewaysSpeed = teamMateSearchSpeed * robotSearchDir;
			}
			else {
				//move ball to correct distance
				forwardSpeed = (ball->distance - ballRotateDistance) * forwardSpeedMult;
				sidewaysSpeed = ball->distanceX * sidewaysSpeedMult;
			}
		}

		robot->setTargetDir(forwardSpeed, sidewaysSpeed);
		robot->lookAt(ball);

	}
}

void TeamController::FindBallState::onEnter(Robot* robot, Parameters parameters) {
	//TODO fill this out
}

void TeamController::FindBallState::step(float dt, Vision::Results* visionResults, Robot* robot, float totalDuration, float stateDuration, float combinedDuration) {
	Object* ball = visionResults->getClosestBall();

	if (robot->dribbler->gotBall()) {
		Parameters parameters;
		parameters["next-state"] = "find-ball";
		parameters["target-type"] = "enemy-goal";
		parameters["kick-type"] = "chip";
		parameters["last-state"] = "find-front";
		ai->setState("aim-kick", parameters);
	}
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
	Object* ball = visionResults->getClosestBall(Dir::FRONT);

	if (robot->dribbler->gotBall()) {
		Parameters parameters;
		parameters["next-state"] = "find-ball";
		parameters["target-type"] = "enemy-goal";
		parameters["kick-type"] = "chip";
		parameters["last-state"] = "fetch-ball-front";
		ai->setState("aim-kick", parameters);
	}

	if (ball == NULL) {
		Parameters parameters;
		parameters["next-state"] = "fetch-ball";
		ai->setState("find-ball", parameters);
	}
	else {
		//configuration parameters
		float forwardSpeedMultiplier = 1.0f;
		float sidewaysSpeedMultiplier = 1.0f;

		float forwardSpeed = 0.0f, sidewaysSpeed = 0.0f;

		forwardSpeed = ball->distanceY * forwardSpeedMultiplier;
		sidewaysSpeed = ball->distanceY * sidewaysSpeedMultiplier;

		robot->setTargetDir(forwardSpeed, sidewaysSpeed);
		robot->lookAt(ball);
	}
}

void TeamController::FetchBallRearState::onEnter(Robot* robot, Parameters parameters) {
	//TODO fill this out
}

void TeamController::FetchBallRearState::step(float dt, Vision::Results* visionResults, Robot* robot, float totalDuration, float stateDuration, float combinedDuration) {
	//TODO fill this out
	//temporary substitute
	robot->setTargetOmega(1.0f);
	Object* ball = visionResults->getClosestBall(Dir::FRONT);
	if (ball != NULL) {
		ai->setState("fetch-ball-front");
		return;
	}
}

void TeamController::DriveToOwnGoalState::onEnter(Robot* robot, Parameters parameters) {
	droveTowardEnemyGoal = false;
	droveTowardFriendlyGoal = false;
	searchedEnemyGoal = false;
	goalSearchDir = 1.0f;
}

void TeamController::DriveToOwnGoalState::step(float dt, Vision::Results* visionResults, Robot* robot, float totalDuration, float stateDuration, float combinedDuration) {

	Object* ownGoal = visionResults->getLargestGoal(ai->defendSide, Dir::REAR);
	Object* enemyGoal = visionResults->getLargestGoal(ai->targetSide, Dir::FRONT);
	bool shouldSearchEnemyGoal = false;
	bool shouldDriveTowardEnemyGoal = false;

	//configuration parameters
	float goalSearchOmega = 4.0f;
	float enemyGoalAngleMultiplier = 0.2f;
	float ownGoalDistanceYMultiplier = 0.5f;
	float ownGoalTargetDistanceY = 0.15f;
	float cantSeeEnemyGoalDistanceYThreshold = 0.5f;
	float enemyGoalDistanceTarget = 3.0f;
	float enemyGoalSpeedMultiplier = 1.0f;
	float enemyGoalOmegaMultiplier = 1.0f;
	float ownGoalDistanceErrorThreshold = 0.15f;
	float enemyGoalAngleErrorThreshold = Math::PI / 60.0f;

	shouldSearchEnemyGoal = droveTowardFriendlyGoal && !droveTowardEnemyGoal && enemyGoal == NULL && !searchedEnemyGoal;
	shouldDriveTowardEnemyGoal = droveTowardFriendlyGoal && !droveTowardEnemyGoal && enemyGoal != NULL && ownGoal == NULL;

	if (robot->hasTasks()) {
		robot->clearTasks();
	}
	if (shouldSearchEnemyGoal) {
		//start searching for enemy goal
		robot->setTargetDir(0.0f, 0.0f);
		robot->setTargetOmega(goalSearchDir * goalSearchOmega);
	}
	else if (shouldDriveTowardEnemyGoal) {
		searchedEnemyGoal = true;
		//start driving toward enemy goal
		robot->setTargetDir(Math::Rad(enemyGoal->angle), enemyGoal->distance * enemyGoalSpeedMultiplier, enemyGoal->angle * enemyGoalOmegaMultiplier);
		if (enemyGoal->distance < enemyGoalDistanceTarget) {
			droveTowardEnemyGoal = true;
		}

	}
	else if (ownGoal == NULL) {
		//start searching for own goal
		robot->setTargetOmega(goalSearchOmega * goalSearchDir);
	}
	else {
		float ownGoalDistanceYerror = 999.0f;
		float enemyGoalAngleerror = 999.0f;
		//start driving in front of own goal
		float forwardSpeed = 0.0f;
		float sideWaysSpeed = 0.0f;

		//std::cout << ownGoal->distanceY;

		ownGoalDistanceYerror = ownGoal->distanceY - ownGoalTargetDistanceY;

		forwardSpeed = (ownGoal->distanceY - ownGoalTargetDistanceY) * -ownGoalDistanceYMultiplier;
		if (enemyGoal != NULL) {
			sideWaysSpeed = enemyGoal->angle * enemyGoalAngleMultiplier;
			enemyGoalAngleerror = enemyGoal->angle;
		}
		else if (ownGoal->distanceY < cantSeeEnemyGoalDistanceYThreshold) {
			droveTowardFriendlyGoal = true;
		}

		if (abs(ownGoalDistanceYerror) < ownGoalDistanceErrorThreshold && abs(enemyGoalAngleerror) < enemyGoalAngleErrorThreshold) {
			//position set
			//ai->setState("manual-control");
			ai->setState("defend-goal");
			return;
		}

		robot->setTargetDir(forwardSpeed, sideWaysSpeed);
		robot->lookAtBehind(ownGoal);
	}
}

void TeamController::AimKickState::onEnter(Robot* robot, Parameters parameters) {
	nextState = "manual-control";
	lastState = "manual-control";
	kickType = "pass";
	targetType = "team-robot";
	if (parameters.find("next-state") != parameters.end()) {
		nextState = parameters["next-state"];
	}
	if (parameters.find("last-state") != parameters.end()) {
		lastState = parameters["last-state"];
	}
	if (parameters.find("kick-type") != parameters.end()) {
		kickType = parameters["kick-type"];
	}
	if (parameters.find("target-type") != parameters.end()) {
		targetType = parameters["target-type"];
	}
	validCount = 0;
}

void TeamController::AimKickState::step(float dt, Vision::Results* visionResults, Robot* robot, float totalDuration, float stateDuration, float combinedDuration) {
	Object* target;
	if (targetType.compare("team-robot") == 0) target = visionResults->getLargestRobot(ai->teamColor, Dir::FRONT);
	else if (targetType.compare("enemy-goal") == 0) target = visionResults->getLargestGoal(ai->targetSide, Dir::FRONT);
	else target = visionResults->getLargestGoal(ai->targetSide, Dir::FRONT);

	if (!robot->dribbler->gotBall()) {
		robot->dribbler->stop();
		robot->stop();
		ai->setState(lastState);
		return;
	}

	//configuration parameters
	float targetAngleError = Math::PI / 90.0f;
	float targetAngleMultiplier = 0.35f;
	int passStrength = 700;
	int directKickStrength = 4000;
	float chipKickAdjust = -0.1f;
	int validCountThreshold = 2;
	float aimAdjustRobotDistance = 1.2f;

	if (target == NULL) {
		robot->spinAroundDribbler();
		validCount = 0;
	}
	else {
		float targetAngle;
		Object* closestRobot = visionResults->getRobotNearObject(ai->enemyColor, target, Dir::FRONT, aimAdjustRobotDistance);
		if (closestRobot == NULL) {
			targetAngle = target->angle;
		}
		else {
			if (closestRobot->angle > target->angle) {
				targetAngle = visionResults->getObjectPartAngle(target, Part::LEFTSIDE);
			}
			else {
				targetAngle = visionResults->getObjectPartAngle(target, Part::RIGHTSIDE);
			}
		}

		if (abs(targetAngle) < targetAngleError) {
			validCount++;
		}
		else {
			validCount = 0;
		}
		if (validCount > validCountThreshold) {
			robot->stop();
			robot->dribbler->stop();
			if (kickType.compare("pass") == 0) robot->coilgun->kick(passStrength);
			else if (kickType.compare("direct") == 0) robot->coilgun->kick(directKickStrength);
			else if (kickType.compare("chip") == 0) robot->coilgun->chipKick(target->distance + chipKickAdjust);
			ai->setState(nextState);
			return;
		}
		robot->setTargetOmega(targetAngle * -targetAngleMultiplier);
	}
	
}

void TeamController::PassBallState::onEnter(Robot* robot, Parameters parameters) {
	//TODO fill this out
	//send command to teammate to get pass
}

void TeamController::PassBallState::step(float dt, Vision::Results* visionResults, Robot* robot, float totalDuration, float stateDuration, float combinedDuration) {
	//if ball is not in dribbler, change to find ball
	//look for teammate
	//if can see teammate, kick ball toward him
}

void TeamController::GetPassState::onEnter(Robot* robot, Parameters parameters) {
	ai->passNeeded = true;
}

void TeamController::GetPassState::step(float dt, Vision::Results* visionResults, Robot* robot, float totalDuration, float stateDuration, float combinedDuration) {
	Object* ball = visionResults->getClosestBall(Dir::FRONT);
	Object* teamMate = visionResults->getLargestRobot(ai->teamColor, Dir::FRONT);
	if (robot->dribbler->gotBall()) {
		Parameters parameters;
		parameters["next-state"] = "find-ball";
		parameters["target-type"] = "enemy-goal";
		parameters["kick-type"] = "chip";
		parameters["last-state"] = "get-pass";
		ai->setState("aim-kick", parameters);
	}

	if (ball != NULL) {
		robot->lookAt(ball);
	}
	else if (teamMate != NULL) {
		robot->lookAt(teamMate);
	}
	else {
		//configuration parameters
		float searchOmega = 1.0f;

		robot->setTargetDir(0.0f, 0.0f, searchOmega);
	}
}

void TeamController::ManeuverState::onEnter(Robot* robot, Parameters parameters) {
	//TODO fill this out
}

void TeamController::ManeuverState::step(float dt, Vision::Results* visionResults, Robot* robot, float totalDuration, float stateDuration, float combinedDuration) {
	//TODO fill this out
}
