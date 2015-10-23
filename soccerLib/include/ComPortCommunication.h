#ifndef COMPORT_COMMUNICATION_H
#define COMPORT_COMMUNICATION_H

#include "AbstractCommunication.h"
#include "Serial.h"

#include <string>
#include <windows.h>

class ComPortCommunication : public AbstractCommunication {

public:
	struct PortList {
		std::vector<unsigned int> numbers;
		std::vector<std::string> names;
	};

	ComPortCommunication(std::string portName, int baud = 115200);
	~ComPortCommunication();

	static PortList getPortList();
	void send(std::string message);
	bool gotMessages();
	std::string dequeueMessage();
	void close();
	void sync();

private:
	void* run();

	std::string portName;
	int baud;
	std::string partialMessage;
	Messages messages;
	Messages queuedMessages;
	Messages sendQueue;
	char readBuffer[MAX_SIZE];
	char requestBuffer[MAX_SIZE];
	mutable boost::mutex messagesMutex;
	bool receivingMessage;
	bool opened;
	HANDLE commHandle;
};

#endif // COMPORT_COMMUNICATION_H