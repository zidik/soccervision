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
	isCaptain = robot->getConf()->robot.captain;
	if (isCaptain) std::cout << "!!! I AM THE CAPTAIN NOW !!! ";
	friendlyGoalCounter = 0;
	enemyGoalCounter = 0;

	passStrength = 625;
	directKickStrength = 3000;
	chipKickAdjust = 0.5f;
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
	states["find-object"] = new FindObjectState(this);
	states["find-ball-goalkeeper"] = new FindBallGoalkeeperState(this);
	states["fetch-ball-front"] = new FetchBallFrontState(this);
	states["fetch-ball-rear"] = new FetchBallRearState(this);
	states["drive-to-own-goal"] = new DriveToOwnGoalState(this);
	states["aim-kick"] = new AimKickState(this);
	states["pass-ball"] = new PassBallState(this);
	states["get-pass"] = new GetPassState(this);
	states["approach-ball"] = new ApproachBallState(this);
	states["go-to-ball"] = new GoToBallState(this);
	states["find-target"] = new FindTargetState(this);
	states["press-opponent"] = new PressOpponentState(this);
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

				if (command == "KICKOFF-") { currentSituation = GameSituation::KICKOFF; }
				else if (command == "IFREEK--") { currentSituation = GameSituation::INDIRECTFREEKICK; }
				else if (command == "DFREEK--") { currentSituation = GameSituation::DIRECTFREEKICK; }
				else if (command == "GOALK---") { currentSituation = GameSituation::GOALKICK; }
				else if (command == "THROWIN-") { currentSituation = GameSituation::THROWIN; }
				else if (command == "CORNERK-") { currentSituation = GameSituation::CORNERKICK; }
				else if (command == "PENALTY-") { currentSituation = GameSituation::PENALTY; }
				else if (command == "GOAL----") {
					if (commandForOurTeam) friendlyGoalCounter++;
					else enemyGoalCounter++;
				}
			}
			else {
				std::string command = cmd.parameters[0].substr(3);

				if (command == "START----" && isCaptain) {
					std::cout << "Teamcontroller start" << std::endl;
					if (whoHasBall == TeamInPossession::FRIENDLY) {
						switch (currentSituation) {
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
							client->send("run-take-goalkick");
							setState("get-pass");
							std::cout << "- waiting for goal kick" << std::endl;
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
						Parameters parameters;
						parameters["next-state"] = "fetch-ball-front";
						setState("wait-for-kick", parameters);
						client->send("run-defend-goal");
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

float TeamController::getChipKickDistance(float targetDistance) {
	float adjustedDistance = targetDistance - chipKickAdjust;
	float kickDistance = Math::limit(adjustedDistance, 0.4f, 3.5f);
	return kickDistance;
}

void TeamController::WaitForKickState::onEnter(Robot* robot, Parameters parameters) {
	nextState = "manual-control";
	if (parameters.find("next-state") != parameters.end()) {
		nextState = parameters["next-state"];
	}
	
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
		if (startingBallPos.x < -999.0f) {
			startingBallPos.x = ball->distanceX;
			startingBallPos.y = ball->distanceY;

			//std::cout << "Found ball @ position- x:" << ball->distanceX << " y: " << ball->distanceY << std::endl;
		}
		else {
			Math::Vector currentBallPos = Math::Vector(ball->distanceX, ball->distanceY);

			//check if ball has drifted significantly away from starting position or is moving with high enough speed (not using speed currently, speed calculation has too many errors it seems)
			if (abs(currentBallPos.x - startingBallPos.x) > kickDetectionDeltaPosition || abs(currentBallPos.y - startingBallPos.y) > kickDetectionDeltaPosition /*|| ball->relativeMovement.speed > kickDetectionMovingSpeed*/) {
				//kick detected

				Parameters parameters;
				parameters["fetch-style"] = "defensive";
				ai->setState(nextState, parameters);
			}
		}
	}
}

void TeamController::DefendGoalState::onEnter(Robot* robot, Parameters parameters) {
	float maxSideSpeed = 1.0f;
	float inputLimit = 0.75;

	pid.setInputLimits(-inputLimit, inputLimit);
	pid.setOutputLimits(-maxSideSpeed, maxSideSpeed);
	pid.setMode(AUTO_MODE);
	pid.setBias(0.0f);
	pid.setTunings(kP, kI, kD);
	pid.reset();
}

void TeamController::DefendGoalState::step(float dt, Vision::Results* visionResults, Robot* robot, float totalDuration, float stateDuration, float combinedDuration) {

	float goalDistanceTarget = 0.35f;

	if (robot->dribbler->gotBall()) {
		Parameters parameters;
		parameters["next-state"] = "defend-goal";
		parameters["last-state"] = "defend-goal";
		parameters["kick-type"] = "chip";
		parameters["target-type"] = "enemy-goal";
		ai->setState("aim-kick", parameters);

		/*
		if (ai->passNeeded) {
			robot->stop();
			ai->setState("pass-ball", parameters);
		}
		else {
			robot->stop();
			ai->setState("aim-kick", parameters);
		}*/
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
	if (abs(defendedGoal->distance - goalDistanceTarget) > 0.30) {
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

	// pid-based
	pidUpdateCounter++;
	if (pidUpdateCounter % 10 == 0) pid.setInterval(dt);
	pid.setSetPoint(0.0f);
	pid.setProcessValue(ballError);

	float sideSpeed = -pid.compute();
	float sidePower = 0.15;
	if (ball != nullptr) {
		//std::cout << "Distance between robot and ball: " << ball->location.getLength() << std::endl;
		sidePower = Math::map(ball->location.getLength(), 0.0f, 2.0f, 1.0f, 0.15f);
	}
	//std::cout << "Sidepower: " << sidePower << std::endl;
	robot->setTargetDir(goalError * 2.0f, sideSpeed * sidePower);
	robot->lookAtBehind(defendedGoal);


}

void TeamController::InterceptBallState::onEnter(Robot* robot, Parameters parameters) {
	//TODO fill this out
}

void TeamController::InterceptBallState::step(float dt, Vision::Results* visionResults, Robot* robot, float totalDuration, float stateDuration, float combinedDuration) {
	if (robot->dribbler->gotBall()) {
		Parameters parameters;
		parameters["next-state"] = "defend-goal";
		parameters["last-state"] = "defend-goal";
		parameters["kick-type"] = "chip";
		parameters["target-type"] = "enemy-goal";
		ai->setState("aim-kick", parameters);
		/*
		if (ai->passNeeded) {
			ai->setState("pass-ball", parameters);
		}
		else {
			ai->setState("aim-kick", parameters);
		}*/
		return;
	}

	Object* ball = visionResults->getClosestBall(Dir::FRONT);

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
			parameters["last-state"] = "defend-goal";
			parameters["fetch-style"] = "defensive";
			if (!ball->behind) ai->setState("fetch-ball-front", parameters);
			else ai->setState("fetch-ball-behind", parameters);
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

	if (robot->dribbler->gotBall()) {
		Parameters parameters;
		parameters["next-state"] = "drive-to-own-goal";
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
	}
	else {
		Parameters parameters;
		parameters["next-state"] = "drive-to-own-goal";
		parameters["last-state"] = "take-kickoff";
		parameters["kick-type"] = "pass";
		parameters["target-type"] = "team-robot";
		parameters["kick-immediately"] = "Y";
		ai->setState("go-to-ball", parameters);
		return;

	}
}

void TeamController::TakeFreeKickDirectState::onEnter(Robot* robot, Parameters parameters) {
	robot->dribbler->useNormalLimits();
	robot->dribbler->stop();
	ai->client->send("run-defend-goal");
}

void TeamController::TakeFreeKickDirectState::step(float dt, Vision::Results* visionResults, Robot* robot, float totalDuration, float stateDuration, float combinedDuration) {
	Object* ball = visionResults->getClosestBall(Dir::FRONT);

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
		Parameters parameters;
		parameters["next-state"] = "manual-control";
		parameters["last-state"] = "take-freekick-direct";
		parameters["kick-type"] = "chip";
		parameters["target-type"] = "enemy-goal";
		parameters["kick-immediately"] = "N";
		ai->setState("go-to-ball", parameters);
		return;

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
	robot->dribbler->stop();
}

void TeamController::TakePenaltyState::step(float dt, Vision::Results* visionResults, Robot* robot, float totalDuration, float stateDuration, float combinedDuration) {
	Object* ball = visionResults->getClosestBall(Dir::FRONT);

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
		Parameters parameters;
		parameters["next-state"] = "manual-control";
		parameters["last-state"] = "take-penalty";
		parameters["kick-type"] = "direct";
		parameters["target-type"] = "enemy-goal";
		parameters["kick-immediately"] = "Y";
		ai->setState("go-to-ball", parameters);
		return;

	}
}

void TeamController::FindBallState::onEnter(Robot* robot, Parameters parameters) {
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

void TeamController::FindObjectState::onEnter(Robot* robot, Parameters parameters) {
	nextState = "manual-control";
	lastState = "manual-control";
	targetType = "team-robot";
	if (parameters.find("next-state") != parameters.end()) {
		nextState = parameters["next-state"];
	}
	if (parameters.find("last-state") != parameters.end()) {
		lastState = parameters["last-state"];
	}
	if (parameters.find("target-type") != parameters.end()) {
		targetType = parameters["target-type"];
	}
}

void TeamController::FindObjectState::step(float dt, Vision::Results* visionResults, Robot* robot, float totalDuration, float stateDuration, float combinedDuration) {
	Object* target;
	if (targetType.compare("team-robot") == 0) target = visionResults->getLargestRobot(ai->teamColor, Dir::FRONT);
	else if (targetType.compare("enemy-robot") == 0) target = visionResults->getLargestRobot(ai->enemyColor, Dir::FRONT);
	else if (targetType.compare("enemy-goal") == 0) target = visionResults->getLargestGoal(ai->targetSide, Dir::FRONT);
	else if (targetType.compare("team-goal") == 0) target = visionResults->getLargestGoal(ai->defendSide, Dir::FRONT);
	else if (targetType.compare("ball") == 0) target = visionResults->getClosestBall(Dir::FRONT);
	else target = visionResults->getClosestBall(Dir::FRONT);

	if (robot->dribbler->gotBall()) {
		Parameters parameters;
		parameters["next-state"] = "find-object";
		parameters["target-type"] = "enemy-goal";
		parameters["kick-type"] = "chip";
		parameters["last-state"] = "find-object";
		ai->setState("aim-kick", parameters);
	}

	//configuration parameters
	float searchSpeed = 3.0f;

	if (target != NULL) {
		//TODO fix this nonsense, takes a while
		if (target->behind) {
			Parameters parameters;

			ai->setState(nextState);
		}
		else {
			Parameters parameters;

			ai->setState(nextState);
		}
	}
	else {
		robot->setTargetDir(0.0f, 0.0f, searchSpeed);
	}
}

void TeamController::FindBallGoalkeeperState::onEnter(Robot* robot, Parameters parameters) {
	//TODO fill this out
}

void TeamController::FindBallGoalkeeperState::step(float dt, Vision::Results* visionResults, Robot* robot, float totalDuration, float stateDuration, float combinedDuration) {
	//TODO fill this out
}

void TeamController::FetchBallFrontState::onEnter(Robot* robot, Parameters parameters) {
	fetchStyle = FetchStyle::DIRECT;
	nextState = "find-ball";
	lastState = "find-ball";

	if (parameters.find("fetch-style") != parameters.end()) {
		if (parameters["fetch-style"] == "defensive") fetchStyle = FetchStyle::DEFENSIVE;
		else if (parameters["fetch-style"] == "offensive") fetchStyle = FetchStyle::OFFENSIVE;
	}
	if(parameters.find("next-state") != parameters.end()) {
		nextState = parameters["next-state"];
	}
	if (parameters.find("last-state") != parameters.end()) {
		lastState = parameters["last-state"];
	}
	maxSideSpeed = 1.3f;

	pid.setInputLimits(-0.75f, 0.75f);
	pid.setOutputLimits(-maxSideSpeed, maxSideSpeed);
	pid.setMode(AUTO_MODE);
	pid.setBias(0.0f);
	pid.setTunings(kP, kI, kD);
	pid.reset();
}

void TeamController::FetchBallFrontState::step(float dt, Vision::Results* visionResults, Robot* robot, float totalDuration, float stateDuration, float combinedDuration) {
	Object* ball = visionResults->getClosestBall(Dir::FRONT);
	Object* ownGoal = visionResults->getLargestGoal(ai->defendSide, Dir::REAR);
	Object* enemyGoal = visionResults->getLargestGoal(ai->targetSide, Dir::FRONT);

	if (robot->dribbler->gotBall()) {
		Parameters parameters;
		parameters["next-state"] = nextState;
		parameters["target-type"] = "enemy-goal";
		parameters["kick-type"] = "chip";
		parameters["last-state"] = "fetch-ball-front";
		ai->setState("aim-kick", parameters);
	}

	if (ball == NULL) {
		Parameters parameters;
		parameters["next-state"] = "fetch-ball-front";
		ai->setState(lastState, parameters);
	}
	else {
		float ballMinDistance = 1.0f;

		if (ball->distance <= ballMinDistance) {
			robot->dribbler->start();
			robot->dribbler->useNormalLimits();
		}
		else {
			robot->dribbler->stop();
		}

		float ballDistance = ball->getDribblerDistance();

		float maxSideSpeedBallAngle = 20.0f;

		float sidePower = Math::map(Math::abs(Math::radToDeg(ball->angle)), 0.0f, maxSideSpeedBallAngle, 0.0f, 1.0f);

		// pid-based
		pidUpdateCounter++;
		if (pidUpdateCounter % 10 == 0) pid.setInterval(dt);
		pid.setSetPoint(0.0f);
		pid.setProcessValue(ball->distanceX);

		float sideP = -pid.compute();
		float sideSpeed = sideP * sidePower;

		float approachP = Math::map(ball->distance, 0.0f, 1.0f, 0.25f, 1.3f);
		float forwardSpeed = approachP * (1.0f - sidePower);

		// don't move forwards if very close to the ball and the ball is quite far sideways
		if (ballDistance < 0.05f && Math::abs(ball->distanceX) > 0.03f) {
			forwardSpeed = 0.0f;
		}

		robot->setTargetDir(forwardSpeed, sideSpeed);

		switch (fetchStyle) {
		case FetchStyle::DEFENSIVE:
			if (ownGoal != NULL) {
				float lookAtAngle;
				float angleDelta = ownGoal->angle - ball->angle;
				if (angleDelta < 0.0f) angleDelta += Math::TWO_PI;
				float angleDeltaLimit = 45.0f;
				float lookAtAngleMultiplier = Math::map(Math::radToDeg(abs(angleDelta - Math::PI)), 0.0f, angleDeltaLimit, 0.0f, 0.5f);

				lookAtAngle = ownGoal->angle - lookAtAngleMultiplier * (angleDelta - Math::PI);

				robot->lookAtBehind(Math::Rad(lookAtAngle));
			}
			else {
				//turn toward ball slowly, so that its trajectory is more likely to be intercepted
				robot->lookAt(ball, Config::lookAtP / 8.0f);
			}
			break;
		case FetchStyle::DIRECT:
			//turn toward ball slowly, so that its trajectory is more likely to be intercepted
			robot->lookAt(ball, Config::lookAtP / 8.0f);
			break;
		case FetchStyle::OFFENSIVE:
			if (enemyGoal != NULL) {
				float lookAtAngle;
				float angleDelta = enemyGoal->angle - ball->angle;
				float angleDeltaLimit = 45.0f;
				float lookAtAngleMultiplier = Math::map(Math::radToDeg(abs(angleDelta)), 0.0f, angleDeltaLimit, 0.0f, 0.5f);

				lookAtAngle = enemyGoal->angle - lookAtAngleMultiplier * angleDelta;

				robot->lookAt(Math::Rad(lookAtAngle));
			}
			else {
				//turn toward ball slowly, so that its trajectory is more likely to be intercepted
				robot->lookAt(ball, Config::lookAtP / 8.0f);
			}
			break;
		}
		if (ballDistance < 0.15f) {
			robot->lookAt(ball);
		}
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
	float driveSpeed = 1.0f;
	if (parameters.find("speed") != parameters.end()) {
		driveSpeed = Util::toFloat(parameters["speed"]);
	}
	robot->clearTasks();

	if (ai->defendSide == Side::YELLOW) {
		robot->driveTo(0.6f, 1.5f, 0.0f, driveSpeed);
		robot->driveTo(0.35f, 1.5f, 0.0f, driveSpeed);
	}
	else {
		robot->driveTo(Config::fieldWidth - 0.6f, 1.5f, Math::PI, driveSpeed);
		robot->driveTo(Config::fieldWidth - 0.35f, 1.5f, Math::PI, driveSpeed);
	}
	
}

void TeamController::DriveToOwnGoalState::step(float dt, Vision::Results* visionResults, Robot* robot, float totalDuration, float stateDuration, float combinedDuration) {

	if (robot->hasTasks()){
		return;
	}
	ai->setState("defend-goal");
}

void TeamController::AimKickState::onEnter(Robot* robot, Parameters parameters) {
	nextState = "find-ball";
	lastState = "find-ball";
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
		robot->dribbler->start();
		ai->setState(lastState);
		return;
	}

	//configuration parameters
	float targetAngleError = Math::PI / 60.0f;
	float targetAngleMultiplier = 0.35f;
	int validCountThreshold = 3;
	float aimAdjustRobotDistance = 1.2f;
	float robotInMiddleThreshold = Math::PI / 180.0f;
	float maxAimDuration = 2.0f;

	// if aiming has taken too long, perform a weak kick and give up
	if (combinedDuration > maxAimDuration) {
		targetType = "team-robot";
		kickType = "pass";
		ai->client->send("run-get-pass");
	}

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
				if (closestRobot->angle > target->angle) lockedArea = Part::LEFTSIDE;
				else lockedArea = Part::RIGHTSIDE;
				targetAngle = visionResults->getObjectPartAngle(target, lockedArea);
			}
			else if (closestRobot->angle > target->angle) {
				targetAngle = visionResults->getObjectPartAngle(target, Part::LEFTSIDE);
			}
			else {
				targetAngle = visionResults->getObjectPartAngle(target, Part::RIGHTSIDE);
			}
		}

		//average filtering
		int numberOfSamples = 4;
		targetAngleBuffer.push_back(targetAngle);
		while (targetAngleBuffer.size() > numberOfSamples) targetAngleBuffer.erase(targetAngleBuffer.begin());
		float targetAngleSum = 0.0f;
		for (std::vector<float>::iterator it = targetAngleBuffer.begin(); it != targetAngleBuffer.end(); it++) targetAngleSum += *it;
		targetAngle = targetAngleSum / targetAngleBuffer.size();

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
				robot->kick(ai->passStrength);
				ai->setState(nextState);
				robot->dribbler->useNormalLimits();
				return;
			}
			else if (kickType.compare("direct") == 0) {
				robot->kick(ai->directKickStrength);
				ai->setState(nextState);
				robot->dribbler->useNormalLimits();
				return;
			}
			else if (kickType.compare("chip") == 0) {
				robot->dribbler->useChipKickLimits();

				if (robot->dribbler->isRaised()) {
					robot->coilgun->chipKick(ai->getChipKickDistance(target->distance));
					ai->setState(nextState);
					robot->dribbler->useNormalLimits();
					return;
				}
			}
			else {
				ai->client->send("run-fetch-ball-front");
				robot->kick(ai->passStrength);
				ai->setState(nextState);
				robot->dribbler->useNormalLimits();
				return;
			}

		}
		robot->setTargetDir(0.0f, 0.0f);
		robot->lookAt(Math::Rad(targetAngle));
		robot->dribbler->start();
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
	kickImmediately = false;
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
	if (parameters.find("kick-immediately") != parameters.end()) {
		if (parameters["kick-immediately"] == "Y") kickImmediately = true;
		else if (parameters["kick-immediately"] == "N") kickImmediately = false;
	}

	//reset runtime parameters
	validCount = 0;
	areaLocked = false;
	lockedArea = Part::MIDDLE;
	targetAngleBuffer.clear();

	maxSideSpeed = 0.5f;

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
		if (kickType.compare("pass") == 0) ai->client->send("run-fetch-ball-front");
		ai->setState(nextState);
	}

	robot->stop();

	if (!kickImmediately) {
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

	if (!kickImmediately && ball->distance < 0.5f) {
		robot->dribbler->start();
	}

	//kick parameters

	if (kickImmediately) {
		if (kickType.compare("chip") == 0) {
			robot->coilgun->kickOnceGotBall(0, 0, ai->getChipKickDistance(target->distance), 0);
		}
		else if (kickType.compare("pass") == 0) {
			robot->coilgun->kickOnceGotBall(ai->passStrength, 0, 0, 0);
		}
		else if (kickType.compare("direct") == 0) {
			robot->coilgun->kickOnceGotBall(ai->directKickStrength, 0, 0, 0);
		}
		else {
			robot->coilgun->kickOnceGotBall(ai->passStrength, 0, 0, 0);
		}
	}

	// configuration parameters
	float robotInMiddleThreshold = Math::PI / 90.0f;
	float aimAdjustRobotDistance = 1.2f;
	float maxSideSpeedBallAngle = 25.0f;

	float sidePower = Math::map(Math::abs(Math::radToDeg(ball->angle)), 0.0f, maxSideSpeedBallAngle, 0.0f, 1.0f);

	// pid-based
	pidUpdateCounter++;
	if (pidUpdateCounter % 10 == 0) pid.setInterval(dt);
	pid.setSetPoint(0.0f);
	pid.setProcessValue(Math::radToDeg(ball->angle));

	float sideP = -pid.compute();
	float sideSpeed = sideP * sidePower;

	float approachP = Math::map(ball->distance, 0.0f, 1.0f, 0.0625f, 0.375f);
	float forwardSpeed = approachP * (1.0f - sidePower);

	// don't move forwards if very close to the ball and the ball is quite far sideways
	if (ballDistance < 0.05f && Math::abs(ball->distanceX) > 0.015f) {
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
			if (closestRobot->angle > target->angle) lockedArea = Part::LEFTSIDE;
			else lockedArea = Part::RIGHTSIDE;
			targetAngle = visionResults->getObjectPartAngle(target, lockedArea);
		}
		else if (closestRobot->angle > target->angle) {
			targetAngle = visionResults->getObjectPartAngle(target, Part::LEFTSIDE);
		}
		else {
			targetAngle = visionResults->getObjectPartAngle(target, Part::RIGHTSIDE);
		}
	}

	//averages target angles for smoother robot
	targetAngleBuffer.push_back(targetAngle);

	while (targetAngleBuffer.size() > 4) targetAngleBuffer.erase(targetAngleBuffer.begin());

	float targetAngleSum = 0.0f;

	for (std::vector<float>::iterator it = targetAngleBuffer.begin(); it != targetAngleBuffer.end(); it++) targetAngleSum += *it;
	targetAngle = targetAngleSum / targetAngleBuffer.size();

	robot->setTargetDir(forwardSpeed, sideSpeed);
	robot->lookAt(Math::Rad(targetAngle), Config::lookAtP);

	ai->dbg("forwardSpeed", forwardSpeed);
	ai->dbg("sideP", sideP);
	ai->dbg("sidePower", sidePower);
	ai->dbg("sideSpeed", sideSpeed);
	ai->dbg("ballAngle", (Math::radToDeg(ball->angle)));
	ai->dbg("targetAngle", (Math::radToDeg(targetAngle)));
	ai->dbg("ball->distanceX", ball->distanceX);
}

void TeamController::GoToBallState::onEnter(Robot* robot, Parameters parameters) {
	robot->dribbler->useNormalLimits();

	//read input parameters
	nextState = "manual-control";
	lastState = "manual-control";
	kickType = "pass";
	targetType = "team-robot";
	kickImmediately = false;
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
	if (parameters.find("kick-immediately") != parameters.end()) {
		if (parameters["kick-immediately"] == "Y") kickImmediately = true;
		else if (parameters["kick-immediately"] == "N") kickImmediately = false;
	}
}

void TeamController::GoToBallState::step(float dt, Vision::Results* visionResults, Robot* robot, float totalDuration, float stateDuration, float combinedDuration) {
	Object* ball = visionResults->getClosestBall(Dir::FRONT);
	Object* target;

	if (targetType.compare("team-robot") == 0) {
		target = visionResults->getLargestRobot(ai->teamColor, Dir::FRONT);
	}
	else if (targetType.compare("enemy-goal") == 0) {
		target = visionResults->getLargestGoal(ai->targetSide, Dir::FRONT);
	}
	else target = visionResults->getLargestGoal(ai->targetSide, Dir::FRONT);

	if (robot->dribbler->gotBall()) {
		Parameters parameters;
		parameters["next-state"] = nextState;
		parameters["last-state"] = lastState;
		parameters["kick-type"] = kickType;
		parameters["target-type"] = targetType;
		ai->setState("aim-kick", parameters);
		//robot->dribbler->stop();
		//robot->stop();
		return;
	}

	if (ball == NULL) {
		Parameters parameters;
		parameters["next-state"] = lastState;
		ai->setState("find-ball", parameters);
	}
	else {
		//configuration parameters
		float ballRotateDistance = 0.25f;
		float goalAngleError = Math::PI / 30.0f;
		float ballCloseEnoughThreshold = 0.4f;
		float maxSideSpeedBallAngle = 35.0f;
		float maxSideSpeedDistanceX = 1.0f;
		float maxSideSpeed = 0.75f;

		if (target != NULL) {

			if (abs(target->angle) < goalAngleError && ball->distance < ballCloseEnoughThreshold) {
				Parameters parameters;
				parameters["next-state"] = nextState;
				parameters["last-state"] = lastState;
				parameters["kick-type"] = kickType;
				parameters["target-type"] = targetType;
				parameters["kick-immediately"] = kickImmediately ? "Y" : "N";
				ai->setState("approach-ball", parameters);
				return;
			}
			else {
				Parameters parameters;
				parameters["next-state"] = nextState;
				parameters["last-state"] = lastState;
				parameters["kick-type"] = kickType;
				parameters["target-type"] = targetType;
				parameters["kick-immediately"] = kickImmediately ? "Y" : "N";
				ai->setState("find-target", parameters);
			}
		}
		else {
			if (ball->distance < ballRotateDistance) {
				Parameters parameters;
				parameters["next-state"] = nextState;
				parameters["last-state"] = lastState;
				parameters["kick-type"] = kickType;
				parameters["target-type"] = targetType;
				parameters["kick-immediately"] = kickImmediately ? "Y" : "N";
				ai->setState("find-target", parameters);
			}
			else {


				float sidePower = Math::map(Math::abs(Math::radToDeg(ball->angle)), 0.0f, maxSideSpeedBallAngle, 0.0f, 1.0f);

				float sideP = Math::map(ball->distanceX, -maxSideSpeedDistanceX, maxSideSpeedDistanceX, -maxSideSpeed, maxSideSpeed);
				float sideSpeed = sideP * sidePower;

				float approachP = Math::map(ball->distance, 0.0f, 1.0f, 0.0625f, 0.75f);
				float forwardSpeed = approachP * (1.0f - sidePower);

				robot->setTargetDir(forwardSpeed, sideSpeed);
				robot->lookAt(ball);
			}
		}
	}
}

void TeamController::FindTargetState::onEnter(Robot* robot, Parameters parameters) {
	robot->dribbler->useNormalLimits();

	//read input parameters
	nextState = "manual-control";
	lastState = "manual-control";
	kickType = "pass";
	targetType = "team-robot";
	kickImmediately = false;
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
	if (parameters.find("kick-immediately") != parameters.end()) {
		if (parameters["kick-immediately"] == "Y") kickImmediately = true;
		else if (parameters["kick-immediately"] == "N") kickImmediately = false;
	}
}

void TeamController::FindTargetState::step(float dt, Vision::Results* visionResults, Robot* robot, float totalDuration, float stateDuration, float combinedDuration) {
	Object* ball = visionResults->getClosestBall(Dir::FRONT);
	Object* target;

	if (targetType.compare("team-robot") == 0) {
		target = visionResults->getLargestRobot(ai->teamColor, Dir::FRONT);
	}
	else if (targetType.compare("enemy-goal") == 0) {
		target = visionResults->getLargestGoal(ai->targetSide, Dir::FRONT);
	}
	else target = visionResults->getLargestGoal(ai->targetSide, Dir::FRONT);

	if (robot->dribbler->gotBall()) {
		Parameters parameters;
		parameters["next-state"] = nextState;
		parameters["last-state"] = lastState;
		parameters["kick-type"] = kickType;
		parameters["target-type"] = targetType;
		ai->setState("aim-kick", parameters);
		//robot->dribbler->stop();
		//robot->stop();
		return;
	}

	if (ball == NULL) {
		Parameters parameters;
		parameters["next-state"] = lastState;
		ai->setState("find-ball", parameters);
	}
	else {
		float forwardSpeed = 0.0f;
		float sidewaysSpeed = 0.0f;
		float lookAtAngle = 0.0f;
		float targetAngle;

		//configuration parameters
		float ballRotateDistance = 0.25f;
		float ballTooFarDistance = 0.5f;
		float ballNotInMiddleThreshold = Math::PI / 18.0f;
		float targetAngleError = Math::PI / 30.0f;
		float ballCloseEnoughThreshold = 0.4f;
		float maxSideSpeedBallAngle = 35.0f;
		float maxSideSpeedDistanceX = 1.0f;
		float maxSideSpeed = 0.75f;
		float robotInMiddleThreshold = Math::PI / 90.0f;

		if (target != NULL) {

			Object* closestRobot = visionResults->getRobotNearObject(ai->enemyColor, target, Dir::FRONT);
			if (areaLocked) {
				targetAngle = visionResults->getObjectPartAngle(target, lockedArea);
			}
			else if (closestRobot == NULL) {
				targetAngle = target->angle;
			}
			else {
				if (abs(closestRobot->angle) < robotInMiddleThreshold) {
					areaLocked = true;
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

			if (abs(targetAngle) < targetAngleError && ball->distance < ballCloseEnoughThreshold) {
				Parameters parameters;
				parameters["next-state"] = nextState;
				parameters["last-state"] = lastState;
				parameters["kick-type"] = kickType;
				parameters["target-type"] = targetType;
				parameters["kick-immediately"] = kickImmediately ? "Y" : "N";
				ai->setState("approach-ball", parameters);
				return;
			}
			else {
				//turn toward target
				bool spinDirection;
				if (targetAngle > 0.0f) spinDirection = false;
				else spinDirection = true;
				float spinPeriod = Math::map(abs(targetAngle), 0.0f, Math::PI / 3.0f, 15.1f, 2.8f);
				ai->dbg("spinPeriod", spinPeriod);
				ai->dbg("spinDirection", spinDirection);
				robot->spinAroundObject(ball, spinDirection, spinPeriod, ballRotateDistance);
			}
		}
		else {
			if (ball->distance > ballTooFarDistance) {
				Parameters parameters;
				parameters["next-state"] = nextState;
				parameters["last-state"] = lastState;
				parameters["kick-type"] = kickType;
				parameters["target-type"] = targetType;
				parameters["kick-immediately"] = kickImmediately ? "Y" : "N";
				ai->setState("go-to-ball", parameters);
			}

			if (abs(ball->angle) > ballNotInMiddleThreshold) {
				robot->stop();
				robot->lookAt(ball);
			}
			else {
				robot->spinAroundObject(ball, false, 2.8f, ballRotateDistance);
			}
		}
	}
}

void TeamController::PressOpponentState::onEnter(Robot* robot, Parameters parameters) {
	maxSideSpeed = 1.3f;

	pid.setInputLimits(-0.75f, 0.75f);
	pid.setOutputLimits(-maxSideSpeed, maxSideSpeed);
	pid.setMode(AUTO_MODE);
	pid.setBias(0.0f);
	pid.setTunings(kP, kI, kD);
	pid.reset();
}

void TeamController::PressOpponentState::step(float dt, Vision::Results* visionResults, Robot* robot, float totalDuration, float stateDuration, float combinedDuration) {
	Object* enemyRobot = visionResults->getLargestRobot(ai->enemyColor, Dir::FRONT);
	Object* ownGoal = visionResults->getLargestGoal(ai->defendSide, Dir::REAR);
	Object* ball = visionResults->getClosestBall(Dir::FRONT);

	if (robot->dribbler->gotBall()) {
		Parameters parameters;
		parameters["next-state"] = "find-ball";
		parameters["target-type"] = "enemy-goal";
		parameters["kick-type"] = "chip";
		parameters["last-state"] = "press-opponent";
		ai->setState("aim-kick", parameters);
	}

	if (enemyRobot == NULL) {
		Parameters parameters;
		parameters["next-state"] = "press-opponent";
		parameters["target-type"] = "enemy-robot";
		ai->setState("find-object", parameters);
	}
	else {

		if (ball != NULL && ball->distance < 0.5f) {
			Parameters parameters;
			parameters["fetch-style"] = "defensive";
			ai->setState("fetch-ball-front", parameters);
		}

		float pressingDistance = 0.25f;
		float searchForGoalDistance = 0.35f;
		float ballMinDistance = 1.0f;

		float maxSideSpeedRobotAngle = 15.0f;

		float sidePower = Math::map(Math::abs(Math::radToDeg(enemyRobot->angle)), 0.0f, maxSideSpeedRobotAngle, 0.0f, 1.0f);

		// pid-based
		pidUpdateCounter++;
		if (pidUpdateCounter % 10 == 0) pid.setInterval(dt);
		pid.setSetPoint(0.0f);
		pid.setProcessValue(enemyRobot->distanceX);

		float sideP = -pid.compute();
		float sideSpeed = sideP * sidePower;

		float approachP = Math::map(enemyRobot->distance - pressingDistance, -0.5f, 0.5f, -1.25f, 1.25f);
		float forwardSpeed = approachP * (1.0f - sidePower);

		robot->setTargetDir(forwardSpeed, sideSpeed);

		if (ownGoal != NULL) {
			float lookAtAngle;
			float angleDelta = ownGoal->angle - enemyRobot->angle;
			if (angleDelta < 0.0f) angleDelta += Math::TWO_PI;
			float angleDeltaLimit = 45.0f;
			float lookAtAngleMultiplier = Math::map(Math::radToDeg(abs(angleDelta - Math::PI)), 0.0f, angleDeltaLimit, 0.0f, 0.5f);

			lookAtAngle = ownGoal->angle - lookAtAngleMultiplier * (angleDelta - Math::PI);

			robot->lookAtBehind(Math::Rad(lookAtAngle));
		}
		else {
			//turn toward ball slowly, so that its trajectory is more likely to be intercepted
			robot->lookAt(enemyRobot, Config::lookAtP / 8.0f);

			if (enemyRobot->distance < searchForGoalDistance) robot->spinAroundObject(enemyRobot, false, 2.3f, pressingDistance);
		}
	}
}

void TeamController::ManeuverState::onEnter(Robot* robot, Parameters parameters) {
	//TODO fill this out
}

void TeamController::ManeuverState::step(float dt, Vision::Results* visionResults, Robot* robot, float totalDuration, float stateDuration, float combinedDuration) {
	//TODO fill this out
}
