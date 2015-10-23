#include "ComPortCommunication.h"
#include "enumser.h"

ComPortCommunication::ComPortCommunication(std::string portName, int baud) :
portName(portName),
baud(baud),
receivingMessage(false),
opened(false)
{
	// http://support.microsoft.com/kb/115831
	if (portName.size() > 4) {
		portName = "\\\\.\\" + portName;
	}

	std::cout << "! Starting com port communication to " << portName << " @ " << baud << std::endl;

	//commHandle = CreateFile(portName.c_str(), GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, 0, NULL);
	commHandle = CreateFile(portName.c_str(), GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, 0, NULL);

	if (commHandle != INVALID_HANDLE_VALUE) {
		// set timeouts
		COMMTIMEOUTS cto = { MAXDWORD, 0, 0, 0, 0 };
		DCB dcb;

		if (!SetCommTimeouts(commHandle, &cto)) {
			std::cout << "- Could not set com port time-outs" << std::endl;

			close();

			return;
		}

		// set DCB
		memset(&dcb, 0, sizeof(dcb));
		dcb.DCBlength = sizeof(dcb);
		dcb.BaudRate = baud;
		dcb.fBinary = 1;
		dcb.fDtrControl = DTR_CONTROL_ENABLE;
		dcb.fRtsControl = RTS_CONTROL_ENABLE;

		dcb.Parity = NOPARITY;
		dcb.StopBits = ONESTOPBIT;
		dcb.ByteSize = 8;

		if (!SetCommState(commHandle, &dcb)) {
			std::cout << "- Could not set com port parameters" << std::endl;

			close();

			return;
		}

		opened = true;

		std::cout << "! COM port " << portName << " opened @ " << baud << std::endl;
	} else {
		std::cout << "- Failed to open COM port " << portName << " (" << commHandle << ")" << std::endl;
	}
}

ComPortCommunication::~ComPortCommunication() {
	std::cout << "! Closing communication link.. ";

	close();
	join();

	std::cout << "done!" << std::endl;
}

ComPortCommunication::PortList ComPortCommunication::getPortList() {
	PortList portList;

	CEnumerateSerial::UsingSetupAPI2(portList.numbers, portList.names);

	return portList;
}

void ComPortCommunication::send(std::string message) {
	boost::mutex::scoped_lock lock(messagesMutex);

	sendQueue.push(message);
}

void ComPortCommunication::sync() {

	//return;

	boost::mutex::scoped_lock lock(messagesMutex);

	if (sendQueue.size() == 0) {
		return;
	}

	std::string message = "";
	std::string queuedMessage;
	int messageCount = sendQueue.size();

	while (sendQueue.size() > 0) {
		queuedMessage = sendQueue.front();

		sendQueue.pop();

		message += queuedMessage + "\n";

		//if (queuedMessage.substr(0, 6) != "speeds" && queuedMessage.substr(0, 3) != "adc") {
			std::cout << "SEND: " << queuedMessage << " [" << sendQueue.size() << "]" << std::endl;
		//}
	}

	if (message.size() >= MAX_SIZE) {
		std::cout << "- Too big socket message" << std::endl;

		return;
	}

	if (!opened) {
		std::cout << "QUEUE " << message << std::endl;

		queuedMessages.push(message);

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

	//std::cout << "SEND " << messageCount << " MESSAGES: " << message << std::endl;


	memcpy(requestBuffer, message.c_str(), message.size());
	requestBuffer[message.size()] = 0;

	try {
		DWORD numWritten;
		WriteFile(commHandle, requestBuffer, message.size(), &numWritten, NULL);
		//WriteFile(commHandle, message.c_str(), message.size(), &numWritten, NULL);
	} catch (...) {
		std::cout << "- ComPortCommunication sending " << message << " failed" << std::endl;
	}
}

bool ComPortCommunication::gotMessages() {
	boost::mutex::scoped_lock lock(messagesMutex);

	return messages.size() > 0;
}

std::string ComPortCommunication::dequeueMessage() {
	boost::mutex::scoped_lock lock(messagesMutex);

	int messageCount = messages.size();

	if (messageCount == 0) {
		//std::cout << "R [" << messageCount << "]" << std::endl;

		return "";
	}

	std::string message = messages.front();

	messages.pop();

	//if (message.substr(0, 7) != "<speeds" && message.substr(0, 4) != "<adc") {
		std::cout << "RECV: '" << message << "' [" << (messageCount - 1) << "]" << std::endl;
	//}

	return message;
}

void ComPortCommunication::close() {
	CloseHandle(commHandle);

	opened = false;
}

void* ComPortCommunication::run() {
	std::cout << "! Running COM port thread" << std::endl;

	while (opened) {
		sync();

		DWORD numRead;

		BOOL ret = ReadFile(commHandle, readBuffer, MAX_SIZE - 1, &numRead, NULL);

		if (!ret) {
			continue;
		}

		readBuffer[numRead] = '\0';

		//std::cout << "@ READ " << numRead << " BYTES" << std::endl;

		for (unsigned int i = 0; i < numRead; i++) {
			char c = readBuffer[i];

			if (c == '\n') {
				//std::cout << "@ GOT LF" << std::endl;
			} else {
				if (c == '<') {
					if (receivingMessage) {
						std::cout << "@ GOT '<' BUT WAS ALREADY RECEIVING MESSAGE '" << partialMessage << "', BUFFER: '" << readBuffer << "', THIS SHOULD NOT HAPPEN" << std::endl;
					}

					receivingMessage = true;
				}

				if (receivingMessage) {
					partialMessage += c;
				} else {
					std::cout << "# '" << c << std::endl;
				}

				if (c == '>') {
					if (!receivingMessage) {
						std::cout << "@ GOT '>' BUT WAS NOT RECEIVING MESSAGE, THIS SHOULD NOT HAPPEN" << std::endl;
					}

					receivingMessage = false;

					if (partialMessage.size() > 0) {
						boost::mutex::scoped_lock lock(messagesMutex);

						messages.push(partialMessage);

						//std::cout << "C < " << partialMessage << " [" << messages.size() << "]" << std::endl;

						partialMessage = "";
					} else {
						std::cout << "@ GOT'>' BUT PARTIAL MESSAGE WAS EMPTY, THIS SHOULD NOT HAPPEN" << std::endl;
					}
				}
			}
		}
	}

	return NULL;
}