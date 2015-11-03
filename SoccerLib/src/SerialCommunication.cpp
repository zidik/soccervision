#include "SerialCommunication.h"
#include "Util.h"
#include "enumser.h"

SerialCommunication::SerialCommunication(std::string portName, int baud) :
	portName(portName),
	baud(baud),
	serial(portName, baud),
	receivingMessage(false)
{
	std::cout << "! Starting communication serial to " << portName << " @ " << baud << std::endl;

	serial.setCallback(boost::bind(&SerialCommunication::received, this, _1, _2));
}

SerialCommunication::~SerialCommunication() {
	std::cout << "! Closing communication link.. ";

	close();
	join();

	std::cout << "done!" << std::endl;
}

SerialCommunication::PortList SerialCommunication::getPortList() {
	PortList portList;

	CEnumerateSerial::UsingSetupAPI2(portList.numbers, portList.names);

	return portList;
}

void SerialCommunication::send(std::string message) {
	boost::mutex::scoped_lock lock(messagesMutex);

	sendQueue.push(message);
}

void SerialCommunication::sync() {

//return;

	boost::mutex::scoped_lock lock(messagesMutex);

	std::string message = "";
	std::string queuedMessage;
	int messageCount = sendQueue.size();

	// TODO sends speeds only when the queue is empty
	/*if (sendQueue.size() == 0 && !speedsSent) {
		sendQueue.push("speeds:"
			+ Util::toString(speedFL) + ":"
			+ Util::toString(speedFR) + ":"
			+ Util::toString(speedRL) + ":"
			+ Util::toString(speedRR) + ":"
			+ Util::toString(speedDribbler)
		);

		speedsSent = true;
	}*/

	if (sendQueue.size() == 0) {
		return;
	}

	while (sendQueue.size() > 0) {
		queuedMessage = sendQueue.front();

		sendQueue.pop();

		message += queuedMessage + "\n";

		//if (queuedMessage.substr(0, 6) != "speeds" && queuedMessage.substr(0, 3) != "adc") {
		//	std::cout << "SEND: " << queuedMessage << " [" << sendQueue.size() << "]" << std::endl;
		//}

		// TODO only sends one message at a time, remove the break once serial works again
		//break;
	}

	if (message.size() >= MAX_SIZE) {
		std::cout << "- Too big socket message" << std::endl;

		return;
	}

	if (!serial.isOpen()) {
		std::cout << "QUEUE " << message << std::endl;

		queuedMessages.push(message);

		return;
	} else if (serial.errorStatus()) {
		std::cout << "Error: serial port unexpectedly closed" << std::endl;

		return;
	}

	while (!queuedMessages.empty()) {
		std::cout << "! Socket queue size: " << queuedMessages.size() << std::endl;

		std::string queuedMessage = queuedMessages.front();
		queuedMessages.pop();

		send(queuedMessage);
	}

	/*if (message.substr(0, 6) != "speeds") {
		std::cout << "SEND > " << message << std::endl;
	}*/

	//message += "\n";

	//std::cout << "SEND: " << message << " [" << sendQueue.size() << "]" << std::endl;

	memcpy(requestBuffer, message.c_str(), message.size());
	requestBuffer[message.size()] = 0;

	try {
		serial.write(requestBuffer, message.size());
	}
	catch (std::exception& e) {
		std::cout << "- SerialCommunication send error: " << e.what() << std::endl;
	}
}

bool SerialCommunication::gotMessages() {
	boost::mutex::scoped_lock lock(messagesMutex);

	return messages.size() > 0;
}

std::string SerialCommunication::dequeueMessage() {
	boost::mutex::scoped_lock lock(messagesMutex);

	int messageCount = messages.size();

	if (messageCount == 0) {
		//std::cout << "R [" << messageCount << "]" << std::endl;

		return "";
	}

	std::string message = messages.front();

	messages.pop();

	//if (message.substr(0, 7) != "<speeds" && message.substr(0, 4) != "<adc") {
	//	std::cout << "RECV: '" << message << "' [" << (messageCount - 1) << "]" << std::endl;
	//}

	return message;
}

void SerialCommunication::close() {
	serial.close();
}

void* SerialCommunication::run() {
	/*while (serial.isOpen()) {
		sync();

		Util::sleep(16);
		//Util::sleep(50);
	}*/

	return NULL;
}

void SerialCommunication::received(const char *data, unsigned int len) {
	std::vector<char> v(data, data + len);

	for (unsigned int i = 0; i<v.size(); i++) {
		if (v[i] == '\n') {
			
		} else {
			if (v[i] == '<') {
				if (receivingMessage) {
					std::cout << "@ GOT '<' BUT WAS ALREADY RECEIVING MESSAGE, THIS SHOULD NOT HAPPEN" << std::endl;
				}

				receivingMessage = true;
			}

			if (receivingMessage) {
				partialMessage += v[i];
			} else {
				std::cout << "# '" << v[i] << std::endl;
			}

			if (v[i] == '>') {
				if (!receivingMessage) {
					std::cout << "@ GOT '>' BUT WAS NOT RECEIVING MESSAGE, THIS SHOULD NOT HAPPEN" << std::endl;
				}

				receivingMessage = false;

				if (partialMessage.size() > 0) {
					boost::mutex::scoped_lock lock(messagesMutex);

					messages.push(partialMessage);

					//std::cout << "C < " << partialMessage << " [" << messages.size() << "]" << std::endl;

					partialMessage = "";
				}
				else {
					std::cout << "@ GOT '<' BUT PARTIAL MESSAGE WAS EMPTY, THIS SHOULD NOT HAPPEN" << std::endl;
				}
			}
		}
	}

	/*if (partialMessage.size() > 0) {
		std::cout << "P < " << partialMessage << std::endl;
	}*/
}