#include "TeamController.h"

#include "Robot.h"
#include "Dribbler.h"
#include "Coilgun.h"
#include "Command.h"

void TeamController::WaitForKickState::onEnter(Robot* robot, Parameters parameters) {
	//TO-DO overwrite from parameters which team has possession, what game situation it is, ball location
}

void TeamController::WaitForKickState::step(float dt, Vision::Results* visionResults, Robot* robot, float totalDuration, float stateDuration, float combinedDuration) {
	//TO-DO write logic here
}