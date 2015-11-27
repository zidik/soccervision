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
	states["manual-control"] = new TestController::ManualControlState(this);
	states["drive-to"] = new TestController::DriveToState(this);
	states["turn-by"] = new TestController::TurnByState(this);
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
	states["approach-ball"] = new ApproachBallState(this);
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
							std::cout << "- taking kickoff" << std::endl;
							return;
						case GameSituation::INDIRECTFREEKICK:
							setState("take-freekick-indirect");
							std::cout << "- taking indirect free kick" << std::endl;
							return;
						case GameSituation::DIRECTFREEKICK:
							setState("take-freekick-direct");
							std::cout << "- taking direct free kick" << std::endl;
							return;
						case GameSituation::GOALKICK:
							setState("take-goalkick");
							std::cout << "- taking goal kick" << std::endl;
							return;
						case GameSituation::THROWIN:
							setState("take-throwin");
							std::cout << "- taking throw-in" << std::endl;
							return;
						case GameSituation::CORNERKICK:
							setState("take-cornerkick");
							std::cout << "- taking corner kick" << std::endl;
							return;
						case GameSituation::PENALTY:
							setState("take-penalty");
							std::cout << "- taking penalty" << std::endl;
							return;
						case GameSituation::PLACEDBALL:
							setState("find-ball");
							std::cout << "- its a placed ball" << std::endl;
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
}

void TeamController::DefendGoalState::step(float dt, Vision::Results* visionResults, Robot* robot, float totalDuration, float stateDuration, float combinedDuration) {

	float goalDistanceTarget = 0.35f;

	if (robot->dribbler->gotBall()) {
		Parameters parameters;
		parameters["next-state"] = "defend-goal";
		if (ai->passNeeded) {
			robot->stop();
			ai->setState("pass-ball", parameters);
		}
		else {
			robot->stop();
			ai->setState("aim-kick", parameters);
		}
		return;
	}
    
	Object* defendedGoal = visionResults->getLargestGoal(ai->getDefendSide(), Dir::REAR);
	const BallLocalizer::Ball* ball = robot->ballLocalizer->getClosestBall();

	//if goal is not visible in back camera, switch to driving in front of goal state.
	if (defendedGoal == NULL) {
		robot->stop();
		ai->setState("drive-to-own-goal");
		return;
	}
	if (abs(defendedGoal->distance - goalDistanceTarget) > 0.2) {
		robot->stop();
		ai->setState("drive-to-own-goal");
		return;
	}

	float goalError = goalDistanceTarget - defendedGoal->distance;

	
	float ballError = 0.0f;
	if (ball != nullptr){
		ballError = ball->location.y;
	}

	if (abs(goalError) < 0.05){
		goalError = 0.0f;
	}	
	if (abs(ballError) < 0.05){
		ballError = 0.0f;
	}

	goalError = Math::limit(goalError, 0.8f);
	ballError = Math::limit(ballError, 0.8f);

	robot->setTargetDir(goalError, ballError);
	robot->lookAtBehind(defendedGoal);


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
	float ballMovingSpeedMultiplier = 8.0f;

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

			sidewaysSpeed = ball->distanceX * sidewaysSpeedMultiplier + ball->relativeMovement.dX * ballMovingSpeedMultiplier;

			robot->setTargetDir(forwardSpeed, sidewaysSpeed);
		}
	}

}

void TeamController::TakeKickoffState::onEnter(Robot* robot, Parameters parameters) {
	ai->passNeeded = true;
	robot->dribbler->useNormalLimits();
	ai->client->send("run-get-pass");
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
		float forwardSpeedMult = 2.0f;
		float sidewaysSpeedMult = 0.7f;
		float robotSearchDir = -1.0f;
		float minForwardSpeed = 0.1f;
		float ballRotateDistance = 0.2f;
		float ballDistanceError = 0.08f;
		float teamMateSearchSpeed = 0.6f;
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
	robot->dribbler->useNormalLimits();
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
	robot->dribbler->useNormalLimits();
	areaLocked = false;
	lockedArea = Part::MIDDLE;
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
		float lookAtAngle = 0.0f;

		//configuration parameters
		float forwardSpeedMult = 1.1f;
		float sidewaysSpeedMult = 1.1f;
		float robotSearchDir = -1.0f;
		float minForwardSpeed = 0.1f;
		float ballRotateDistance = 0.2f;
		float ballDistanceError = 0.05f;
		float teamMateSearchSpeed = 0.4f;
		float goalAngleError = Math::PI / 30.0f;
		float goalInMiddleThreshold = Math::PI / 90.0f;

		if (goal != NULL) {

			float targetAngle;
			Object* closestRobot = visionResults->getRobotNearObject(ai->enemyColor, goal, Dir::FRONT);
			if (areaLocked) {
				targetAngle = visionResults->getObjectPartAngle(goal, lockedArea);
			}
			else if (closestRobot == NULL) {
				targetAngle = goal->angle;
			}
			else {
				if (abs(closestRobot->angle) < goalInMiddleThreshold) {
					areaLocked = true;
					//TODO choose locked area based on robots position on the field
					lockedArea = Part::RIGHTSIDE;
					targetAngle = visionResults->getObjectPartAngle(goal, lockedArea);
				}
				else if (closestRobot->angle > goal->angle) {
					targetAngle = visionResults->getObjectPartAngle(goal, Part::LEFTSIDE);
				}
				else {
					targetAngle = visionResults->getObjectPartAngle(goal, Part::RIGHTSIDE);
				}
			}

			if (abs(targetAngle) < goalAngleError) {
				Parameters parameters;
				parameters["next-state"] = "manual-control";
				parameters["last-state"] = "take-penalty";
				parameters["kick-type"] = "direct";
				parameters["target-type"] = "enemy-goal";
				ai->setState("approach-ball", parameters);
				return;
			}
			else {
				//turn toward goal
				sidewaysSpeed = targetAngle * -sidewaysSpeedMult;
				forwardSpeed = (ball->distance - ballRotateDistance) * forwardSpeedMult;
				lookAtAngle = ball->angle;
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
			lookAtAngle = ball->angle;
		}

		robot->setTargetDir(forwardSpeed, sidewaysSpeed);
		robot->lookAt(Math::Rad(lookAtAngle));

	}
}

void TeamController::FindBallState::onEnter(Robot* robot, Parameters parameters) {
	//TODO fill this out
	nextState = "manual-control";	
	if (parameters.find("next-state") != parameters.end()) {
		nextState = parameters["next-state"];
	}
}

void TeamController::FindBallState::step(float dt, Vision::Results* visionResults, Robot* robot, float totalDuration, float stateDuration, float combinedDuration) {
	Object* ball = visionResults->getClosestBall(Dir::FRONT);

	if (robot->dribbler->gotBall()) {
		Parameters parameters;
		parameters["next-state"] = "find-ball";
		parameters["target-type"] = "enemy-goal";
		parameters["kick-type"] = "chip";
		parameters["last-state"] = "find-ball";
		ai->setState("aim-kick", parameters);
	}

	//configuration parameters
	float ballSearchSpeed = 3.0f;

	if (ball != NULL) {
		if (ball->behind) {
			Parameters parameters;

			ai->setState(nextState);
		}
		else {
			Parameters parameters;

			ai->setState(nextState);
		}
	}
	else {
		robot->setTargetDir(0.0f, 0.0f, ballSearchSpeed);
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
		parameters["next-state"] = "fetch-ball-front";
		ai->setState("find-ball", parameters);
	}
	else {
		
		//configuration parameters
		float forwardSpeedMultiplier = 1.0f;
		float sidewaysSpeedMultiplier = 3.0f;

		float forwardSpeed = 0.0f, sidewaysSpeed = 0.0f;

		float ballMinDistance = 0.3f;

		if (ball->distance <= ballMinDistance) {
			robot->dribbler->start();
			robot->dribbler->useNormalLimits();
		} else {
			robot->dribbler->stop();
		}

		forwardSpeed = ball->distanceY * forwardSpeedMultiplier;
		sidewaysSpeed = ball->distanceX * sidewaysSpeedMultiplier;

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
	robot->clearTasks();
	robot->driveTo(0.6f, 1.5f, 0.0f, 0.5f);
	robot->driveTo(0.35f, 1.5f, 0.0f, 0.5f);
}

void TeamController::DriveToOwnGoalState::step(float dt, Vision::Results* visionResults, Robot* robot, float totalDuration, float stateDuration, float combinedDuration) {

	if (robot->hasTasks()){
		return;
	}
	ai->setState("defend-goal");
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

	//reset runtime parameters
	validCount = 0;
	areaLocked = false;
	lockedArea = Part::MIDDLE;
}

void TeamController::AimKickState::step(float dt, Vision::Results* visionResults, Robot* robot, float totalDuration, float stateDuration, float combinedDuration) {
	Object* target;
	if (targetType.compare("team-robot") == 0) target = visionResults->getLargestRobot(ai->teamColor, Dir::FRONT);
	else if (targetType.compare("enemy-goal") == 0) target = visionResults->getLargestGoal(ai->targetSide, Dir::FRONT);
	else target = visionResults->getLargestGoal(ai->targetSide, Dir::FRONT);

	if (!robot->dribbler->gotBall()) {
		//robot->dribbler->stop();
		robot->stop();
		ai->setState(lastState);
		return;
	}

	//configuration parameters
	float targetAngleError = Math::PI / 60.0f;
	float targetAngleMultiplier = 0.35f;
	int passStrength = 900;
	int directKickStrength = 4000;
	float chipKickAdjust = 0.1f;
	int validCountThreshold = 2;
	float aimAdjustRobotDistance = 1.2f;
	float robotInMiddleThreshold = Math::PI / 180.0f;

	if (target == NULL) {
		robot->spinAroundDribbler();
		validCount = 0;
	}
	else {
		float targetAngle;
		Object* closestRobot = visionResults->getRobotNearObject(ai->enemyColor, target, Dir::FRONT, aimAdjustRobotDistance);
		if (areaLocked) {
			targetAngle = visionResults->getObjectPartAngle(target, lockedArea);
		}
		else if (closestRobot == NULL) {
			targetAngle = target->angle;
		}
		else {
			if (abs(closestRobot->angle) < robotInMiddleThreshold) {
				areaLocked = true;
				//TODO choose locked area based on robots position on the field
				lockedArea = Part::RIGHTSIDE;
				targetAngle = visionResults->getObjectPartAngle(target, lockedArea);
			}
			else if (closestRobot->angle > target->angle) {
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
			//robot->dribbler->stop();
			if (kickType.compare("pass") == 0) {
				ai->client->send("run-fetch-ball-front");
				robot->kick(passStrength);
				ai->setState(nextState);
				robot->dribbler->useNormalLimits();
				return;
			}
			else if (kickType.compare("direct") == 0) {
				robot->kick(directKickStrength);
				ai->setState(nextState);
				robot->dribbler->useNormalLimits();
				return;
			}
			else if (kickType.compare("chip") == 0)  {
				if (robot->chipKick(target->distance + chipKickAdjust)) {
					ai->setState(nextState);
					robot->dribbler->useNormalLimits();
					return;
				}
			}
			else {
				robot->kick(passStrength);
				ai->setState(nextState);
				robot->dribbler->useNormalLimits();
				return;
			}

		}
		robot->setTargetDir(0.0f, 0.0f);
		robot->lookAt(Math::Rad(targetAngle));
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

void TeamController::ApproachBallState::onEnter(Robot* robot, Parameters parameters) {
	//read input parameters
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

	//reset runtime parameters
	validCount = 0;
	areaLocked = false;
	lockedArea = Part::MIDDLE;

	maxSideSpeed = 1.0f;

	pid.setInputLimits(-45.0f, 45.0f);
	pid.setOutputLimits(-maxSideSpeed, maxSideSpeed);
	pid.setMode(AUTO_MODE);
	pid.setBias(0.0f);
	pid.setTunings(kP, kI, kD);
	pid.reset();
}

void TeamController::ApproachBallState::step(float dt, Vision::Results* visionResults, Robot* robot, float totalDuration, float stateDuration, float combinedDuration) {
	// wait for the possible reverse out of goal task to finish
	if (robot->hasTasks()) {
		return;
	}

	// make it fall out of this state after kick
	if (robot->coilgun->getTimeSinceLastKicked() <= 0.032f && stateDuration > 0.032f) {
		ai->setState(nextState);
	}

	// goes to aim state when got ball is enabled, otherwise always uses chip kick limits and kicks as soon as dribbler senses ball
	//bool aimMode = true;
	bool aimMode = false;

	robot->stop();

	if (aimMode) {
		robot->dribbler->useNormalLimits();

		if (robot->dribbler->gotBall()) {
			Parameters parameters;
			parameters["next-state"] = nextState;
			parameters["last-state"] = lastState;
			parameters["kick-type"] = kickType;
			parameters["target-type"] = targetType;
			ai->setState("aim-kick", parameters);
			return;
		}
	}
	else {
		robot->dribbler->useChipKickLimits();
	}

	// this can fail when aiming at the goal from the side as it won't see the back edge of the goal and thinks it's closer than it really is
	// return to field if got really close to one of the goals
	Object* closestGoal = visionResults->getLargestGoal(Side::UNKNOWN, Dir::FRONT);

	// back up
	if (closestGoal != NULL && closestGoal->distance < 0.1f && ai->wasNearGoalLately()) {
		robot->setTargetDirFor(-2.0f, 0.0f, 0.0f, 0.5f);

		return;
	}

	Object* ball = visionResults->getClosestBall(Dir::FRONT);
	Object* target;
	if (targetType.compare("team-robot") == 0) target = visionResults->getLargestRobot(ai->teamColor, Dir::FRONT);
	else if (targetType.compare("enemy-goal") == 0) target = visionResults->getLargestGoal(ai->targetSide, Dir::FRONT);
	else target = visionResults->getLargestGoal(ai->targetSide, Dir::FRONT);

	bool usingGhost = false;

	if (ball != NULL) {
		ai->setLastBall(ball);
	}
	else {
		// use ghost ball if available
		ball = ai->getLastBall(Dir::FRONT);

		if (ball != NULL) {
			usingGhost = true;
		}
	}

	ai->dbg("ballVisible", ball != NULL);
	ai->dbg("targetVisible", target != NULL);
	ai->dbg("usingGhost", usingGhost);

	if (target == NULL) {
		// can't see the target, switch to last state
		ai->setState(lastState);
		return;
	}

	// switch to last state if ball not visible any more
	if (ball == NULL) {
		ai->setState(lastState);
		return;
	}

	float ballDistance = ball->getDribblerDistance();
	float ballFarDistance = 1.0f;
	int kickStrength = 0;

	ai->dbg("ballDistance", ballDistance);

	if (ball->distance > ballFarDistance) {
		ai->setState(lastState);
		return;
	}

	if (aimMode && ball->distance < 0.5f) {
		robot->dribbler->start();
	}

	//kick parameters
	int passStrength = 900;
	int directKickStrength = 4000;
	float chipKickAdjust = 0.1f;

	if (!aimMode) {
		if (kickType.compare("chip") == 0) {
			robot->coilgun->kickOnceGotBall(0, 0, target->distance + chipKickAdjust, 0);
		}
		else if (kickType.compare("pass") == 0) {
			robot->coilgun->kickOnceGotBall(passStrength, 0, 0, 0);
		}
		else if (kickType.compare("direct") == 0) {
			robot->coilgun->kickOnceGotBall(directKickStrength, 0, 0, 0);
		}
	}

	// configuration parameters
	float robotInMiddleThreshold = Math::PI / 90.0f;
	float aimAdjustRobotDistance = 1.2f;
	float maxSideSpeedBallAngle = 35.0f;

	float sidePower = Math::map(Math::abs(Math::radToDeg(ball->angle)), 0.0f, maxSideSpeedBallAngle, 0.0f, 1.0f);

	// pid-based
	pid.setSetPoint(0.0f);
	pid.setProcessValue(Math::radToDeg(ball->angle));

	float sideP = -pid.compute();
	float sideSpeed = sideP * sidePower;

	float approachP = Math::map(ball->distance, 0.0f, 1.0f, 0.25f, 1.5f);
	float forwardSpeed = approachP * (1.0f - sidePower);

	// don't move forwards if very close to the ball and the ball is quite far sideways
	if (ballDistance < 0.05f && Math::abs(ball->distanceX) > 0.03f) {
		forwardSpeed = 0.0f;
	}

	float targetAngle;
	Object* closestRobot = visionResults->getRobotNearObject(ai->enemyColor, target, Dir::FRONT, aimAdjustRobotDistance);
	if (areaLocked) {
		targetAngle = visionResults->getObjectPartAngle(target, lockedArea);
	}
	else if (closestRobot == NULL) {
		targetAngle = target->angle;
	}
	else {
		if (abs(closestRobot->angle) < robotInMiddleThreshold) {
			areaLocked = true;
			//TODO choose locked area based on robots position on the field
			lockedArea = Part::RIGHTSIDE;
			targetAngle = visionResults->getObjectPartAngle(target, lockedArea);
		}
		else if (closestRobot->angle > target->angle) {
			targetAngle = visionResults->getObjectPartAngle(target, Part::LEFTSIDE);
		}
		else {
			targetAngle = visionResults->getObjectPartAngle(target, Part::RIGHTSIDE);
		}
	}

	robot->setTargetDir(forwardSpeed, sideSpeed);
	//robot->lookAt(goal, lookAtGoalP);
	robot->lookAt(Math::Rad(targetAngle), Config::lookAtP);

	ai->dbg("forwardSpeed", forwardSpeed);
	ai->dbg("sideP", sideP);
	ai->dbg("sidePower", sidePower);
	ai->dbg("sideSpeed", sideSpeed);
	ai->dbg("ballAngle", (Math::radToDeg(ball->angle)));
	ai->dbg("targetAngle", (Math::radToDeg(targetAngle)));
	ai->dbg("ball->distanceX", ball->distanceX);
}

void TeamController::ManeuverState::onEnter(Robot* robot, Parameters parameters) {
	//TODO fill this out
}

void TeamController::ManeuverState::step(float dt, Vision::Results* visionResults, Robot* robot, float totalDuration, float stateDuration, float combinedDuration) {
	//TODO fill this out
}
