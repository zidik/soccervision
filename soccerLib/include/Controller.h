#ifndef CONTROLLER_H
#define CONTROLLER_H

#include "Vision.h"
#include "AbstractCommunication.h"
#include "Command.h"
#include "Config.h"

#include <string>
#include <map>

class Robot;
class Command;

class Controller : public AbstractCommunication::Listener, public Command::Listener {

public:
	Controller(Robot* robot, AbstractCommunication* com) : robot(robot), com(com) {}
	virtual void onEnter() {}
	virtual void onExit() {}
	virtual bool handleRequest(std::string request) { return false; }
	virtual bool handleCommand(const Command& cmd) { return false; }
	virtual void handleCommunicationMessage(std::string message) {}
	virtual Side getTargetSide() { return Side::UNKNOWN; }
	virtual bool isPlaying() { return false; }
	virtual void step(float dt, Vision::Results* visionResults) = 0;
	virtual std::string getJSON() { return "null"; }

protected:
    Robot* robot;
	AbstractCommunication* com;

};

typedef std::map<std::string, Controller*> ControllerMap;

#endif // CONTROLLER_H
