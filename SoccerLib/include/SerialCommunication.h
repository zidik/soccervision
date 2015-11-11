#ifndef SERIAL_COMMUNICATION_H
#define SERIAL_COMMUNICATION_H

#include "AbstractCommunication.h"
#include "Serial.h"

class SerialCommunication : public AbstractCommunication {

public:
	struct PortList {
		std::vector<unsigned int> numbers;
		std::vector<std::string> names;
	};

	SerialCommunication(std::string portName, int baud = 115200);
	~SerialCommunication();

	static PortList getPortList();
	void send(std::string message);
	bool gotMessages();
	std::string dequeueMessage();
	void close();
	void sync();

private:
	void* run();
	void received(const char *data, unsigned int len);

	CallbackSerial serial;
	std::string portName;
	int baud;
	std::string partialMessage;
	Messages messages;
	Messages queuedMessages;
	Messages sendQueue;
	char requestBuffer[MAX_SIZE];
	mutable boost::mutex messagesMutex;
	bool receivingMessage;
};

#endif // SERIAL_COMMUNICATION_H