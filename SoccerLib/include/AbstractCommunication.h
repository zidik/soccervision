#ifndef ABSTRACT_COMMUNICATION_H
#define ABSTRACT_COMMUNICATION_H

#include "Thread.h"

#include <string>
#include <queue>

class AbstractCommunication : public Thread {

public:
	class Listener {

	public:
		virtual void handleCommunicationMessage(std::string message) = 0;
	};

	typedef std::queue<std::string> Messages;
	enum { MAX_SIZE = 4098 };

	virtual void send(std::string message) = 0;
	virtual bool gotMessages() = 0;
	virtual std::string dequeueMessage() = 0;
	//virtual int start() { return 0; };
	virtual void close() = 0;
	virtual void sync() {};

	// temporary speeds hack
	void setSpeeds(int FL, int FR, int RL, int RR, int dribbler) {
		speedFL = FL;
		speedFR = FR;
		speedRL = RL;
		speedRR = RR;
		speedDribbler = dribbler;
		speedsSent = false;
	}

protected:
	int speedFL;
	int speedFR;
	int speedRL;
	int speedRR;
	int speedDribbler;
	bool speedsSent;

};

#endif // ABSTRACT_COMMUNICATION_H