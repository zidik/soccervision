#include "BaseAI.h"
#include "Robot.h"

BaseAI::BaseAI(Robot* robot, AbstractCommunication* com) : Controller(robot, com), currentState(NULL), currentStateName(""), lastStateName(""), totalDuration(0.0f), currentStateDuration(0.0f), combinedStateDuration(0.0f) {

}

BaseAI::~BaseAI() {
	for (StatesIt it = states.begin(); it != states.end(); it++) {
		delete it->second;
	}

	states.clear();
}

void BaseAI::setState(std::string state) {
	Parameters parameters;

	setState(state, parameters);
}

void BaseAI::setState(std::string state, Parameters parameters) {
	if (states.find(state) == states.end()) {
		std::cout << "- Invalid state '" << state << "' requested" << std::endl;

		return;
	}

	State* newState = states[state];

	if (currentState != NULL) {
		//std::cout << "! Switched state from " << currentStateName << " to " << state << ", lasted: " << currentStateDuration << std::endl;

		currentState->onExit(robot);
	} else {
		//std::cout << "! Set initial AI state to " << state << std::endl;
	}

	currentStateDuration = 0.0f;

	if (state != currentStateName) {
		combinedStateDuration = 0.0f;
		lastStateName = currentStateName;
		currentStateName = state;
	}

	robot->clearTasks();

	newState->onEnter(robot, parameters);

	currentState = newState;
}

void BaseAI::handleCommunicationMessage(std::string message) {
	if (Command::isValid(message)) {
        Command command = Command::parse(message);

		handleCommand(command);
	}
}