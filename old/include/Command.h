#ifndef COMMAND_H
#define COMMAND_H

#include <string>
#include <vector>

class Command {

public:
	class Listener {

	public:
		virtual bool handleCommand(const Command& cmd) = 0;

	};

	typedef std::vector<std::string> Parameters;

    Command(std::string name, Parameters parameters) : name(name), parameters(parameters) {};

    static bool isValid(std::string input);
    static Command parse(std::string input);

    std::string name;
    Parameters parameters;

};

#endif // COMMAND_H
