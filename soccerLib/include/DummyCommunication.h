#ifndef DUMMY_COMMUNICATION_H
#define DUMMY_COMMUNICATION_H

#include "AbstractCommunication.h"

#include <iostream>

class DummyCommunication : public AbstractCommunication {

public:

	DummyCommunication() {};

	void send(std::string message) {
		//std::cout << "SEND: " << message << std::endl;
	};

	bool gotMessages() {
		return false;
	};

	std::string dequeueMessage() {
		return "";
	};

	void close() {};

private:
	void* run() { return NULL; }

};

#endif // DUMMY_COMMUNICATION_H