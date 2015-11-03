#include "ManualController.h"

#include "Robot.h"
#include "Command.h"
#include "Dribbler.h"
#include "Coilgun.h"
#include "Util.h"

ManualController::ManualController(Robot* robot, AbstractCommunication* com) : Controller(robot, com), lastCommandTime(0.0) {

};

void ManualController::reset() {
	com->send("reset");
}

void ManualController::step(float dt, Vision::Results* visionResults) {
	double time = Util::millitime();

	// emergency stop
	if (time - lastCommandTime > 0.5) {
		robot->stop();
	}
}

bool ManualController::handleCommand(const Command& cmd) {
    if (cmd.name == "target-vector" && cmd.parameters.size() == 3) {
        handleTargetVectorCommand(cmd);
    } else if (cmd.name == "target-dir" && cmd.parameters.size() == 3) {
        handleTargetDirCommand(cmd);
    } else if (cmd.name == "set-dribbler" && cmd.parameters.size() == 1) {
        handleSetDribblerCommand(cmd);
    } else if (cmd.name == "kick" && cmd.parameters.size() == 1) {
        handleKickCommand(cmd);
    } else if (cmd.name == "reset-position") {
		robot->setPosition(Config::fieldWidth / 2.0f, Config::fieldHeight / 2.0f, 0.0f);
    } else {
        return false;
    }

    return true;
}

void ManualController::handleCommunicationMessage(std::string message) {
	if (Command::isValid(message)) {
        Command command = Command::parse(message);

		handleCommand(command);
	}
}

void ManualController::handleTargetVectorCommand(const Command& cmd) {
    float x = Util::toFloat(cmd.parameters[0]);
    float y = Util::toFloat(cmd.parameters[1]);
    float omega = Util::toFloat(cmd.parameters[2]);

    robot->setTargetDir(x, y, omega);

	lastCommandTime = Util::millitime();
}

void ManualController::handleTargetDirCommand(const Command& cmd) {
    Math::Deg dir = Math::Deg(Util::toFloat(cmd.parameters[0]));
    float speed = Util::toFloat(cmd.parameters[1]);
    float omega = Util::toFloat(cmd.parameters[2]);

    robot->setTargetDir(dir, speed, omega);

	lastCommandTime = Util::millitime();
}

void ManualController::handleSetDribblerCommand(const Command& cmd) {
    float targetOmega = Util::toFloat(cmd.parameters[0]);

	robot->dribbler->setTargetOmega(targetOmega);
}

void ManualController::handleKickCommand(const Command& cmd) {
    int strength = Util::toInt(cmd.parameters[0]);

	robot->coilgun->kick(strength);
}

std::string ManualController::getJSON() {
	return "null";
}
