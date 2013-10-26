#ifndef BASEAI_H
#define BASEAI_H

#include "Controller.h"
#include "Vision.h"
#include "DebouncedButton.h"
#include "Config.h"

#include <string>
#include <map>

class BaseAI : public Controller {

public:
	typedef std::map<std::string, std::string> Parameters;

	class State {

	public:
		State(BaseAI* ai) : ai(ai) {}
		virtual void onEnter(Robot* robot, Parameters parameters) {}
		virtual void onExit(Robot* robot) {}
		virtual void step(float dt, Vision::Results* visionResults, Robot* robot, float totalDuration, float stateDuration) = 0;

	protected:
		BaseAI* ai;

	};

	typedef std::map<std::string, State*> States;
	typedef States::iterator StatesIt;

	BaseAI(Robot* robot, Communication* com);
	virtual ~BaseAI();
	virtual void setState(std::string state);
	virtual void setState(std::string state, Parameters parameters);
	virtual void handleCommunicationMessage(std::string message);

protected:
	States states;
	State* currentState;
	std::string currentStateName;
	float totalDuration;
	float currentStateDuration;

};

#endif // BASEAI_H