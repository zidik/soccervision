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
	states["wait-for-kick"] = new WaitForKickState(this);
}

void TeamController::WaitForKickState::onEnter(Robot* robot, Parameters parameters) {
	//TO-DO overwrite from parameters which team has possession, what game situation it is, ball location
}

void TeamController::WaitForKickState::step(float dt, Vision::Results* visionResults, Robot* robot, float totalDuration, float stateDuration, float combinedDuration) {
	//TO-DO write logic here
}