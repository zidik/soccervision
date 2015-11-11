#include "EthernetCommunication.h"
#include "Util.h"

#include <boost/bind.hpp>

EthernetCommunication::EthernetCommunication(std::string host, int port) : host(host), port(port), running(false), socket(NULL), receiveBuffer2(boost::asio::buffer(receiveBuffer, MAX_SIZE)) {

}

EthernetCommunication::~EthernetCommunication() {
	std::cout << "! Closing communication link.. ";

	close();
	join();

	std::cout << "done!" << std::endl;

	if (socket != NULL) delete socket; socket = NULL;
}

void EthernetCommunication::send(std::string message) {
	if (message.size() >= MAX_SIZE) {
		std::cout << "- Too big socket message" << std::endl;

		return;
	}

	if (!running) {
		queuedMessages.push(message);

		return;
	}

	while (!queuedMessages.empty()) {
		std::cout << "! Socket queue size: " << queuedMessages.size() << std::endl;

		std::string queuedMessage = queuedMessages.front();
		queuedMessages.pop();

		//if (queuedMessage.substr(0, 6) != "speeds" && queuedMessage.substr(0, 3) != "adc") {
		//	std::cout << "SEND FROM QUEUE: " << queuedMessage << " [" << queuedMessages.size() << "]" << std::endl;
		//}

		send(queuedMessage);
	}

	/*if (message.substr(0, 6) != "speeds" && message.substr(0, 6) != "charge" && message.substr(0, 3) != "adc") {
		// incoming message
		std::cout << "SEND: " << message << std::endl;
	}*/

	message += "\n";

	memcpy(requestBuffer, message.c_str(), message.size());
	requestBuffer[message.size()] = 0;

	//boost::shared_ptr<std::string> requestBuffer(new std::string(message));

	try {
		/*boost::asio::ip::udp::endpoint remoteEndpoint = boost::asio::ip::udp::endpoint(
			boost::asio::ip::address::from_string(host),
			port
		);*/

		/*socket->async_send_to(
			boost::asio::buffer(requestBuffer, message.length()),
			remoteEndpoint,
			//boost::asio::buffer(*requestBuffer), remoteEndpoint,
			boost::bind(
				&EthernetCommunication::onSend,
				this,
				boost::asio::placeholders::error,
				boost::asio::placeholders::bytes_transferred
			)
		);*/

		socket->send_to(
			boost::asio::buffer(requestBuffer, message.length()),
			remoteEndpoint
		);
	} catch (std::exception& e) {
		std::cout << "- EthernetCommunication send error: " << e.what() << std::endl;
	}
}

bool EthernetCommunication::gotMessages() {
	boost::mutex::scoped_lock lock(messagesMutex);

	return messages.size() > 0;
}

std::string EthernetCommunication::dequeueMessage() {
	boost::mutex::scoped_lock lock(messagesMutex);
	
	if (messages.size() == 0) {
		return "";
	}

	std::string message = messages.front();

	messages.pop();

	/*if (message.substr(0, 7) != "<speeds" && message.substr(0, 4) != "<adc") {
		// incoming message
		std::cout << "RECV: " << message << std::endl;
	}*/

	return message;
}

void* EthernetCommunication::run() {
	std::cout << "! Starting communication socket connection to " << host << ":" << port << std::endl;

	running = true;

	socket = new udp::socket(ioService, udp::endpoint(udp::v4(), port));

	remoteEndpoint = boost::asio::ip::udp::endpoint(
		boost::asio::ip::address::from_string(host),
		port
	);

	//receiveBuffer2 = boost::asio::buffer(receiveBuffer, MAX_SIZE);

	receiveNext();

	ioService.run();

	return NULL;
}

void EthernetCommunication::receiveNext() {
	try {
		socket->async_receive_from(
			//boost::asio::buffer(message, MAX_SIZE), endpoint,
			//boost::asio::buffer(receiveBuffer, MAX_SIZE),
			receiveBuffer2,
			endpoint,
			boost::bind(
				&EthernetCommunication::onReceive,
				this,
				boost::asio::placeholders::error,
				boost::asio::placeholders::bytes_transferred
			)
		);
	} catch (std::exception& e) {
		std::cout << "- EthernetCommunication receive error: " << e.what() << std::endl;
	}
}

void EthernetCommunication::onReceive(const boost::system::error_code& error, size_t bytesReceived) {
	if (bytesReceived >= MAX_SIZE) {
		std::cout << "- Too large socket message received: " << bytesReceived << std::endl;
	}

	if ((!error || error == boost::asio::error::message_size) && bytesReceived > 0) {
		std::string msg = std::string(receiveBuffer, bytesReceived);
		//std::string msg = std::string(receiveBuffer.data(), bytesReceived);

		//if (msg.substr(0, 7) != "<speeds") {
			// outgoing message
		//	std::cout << "RECV: " << msg << ", bytesReceived: " << bytesReceived << std::endl;
		//}

		boost::mutex::scoped_lock lock(messagesMutex);
		messages.push(msg);
	} else if (error.value() != 995) {
		std::cout << "- Socket receive error: " << error << ", bytesReceived: " << bytesReceived << std::endl;
	}

	if (running) {
		receiveNext();
	}
}

void EthernetCommunication::onSend(const boost::system::error_code& error, size_t bytesSent) {
	if (error) {
		std::cout << "- Socket send error: " << error << ", bytesSent: " << bytesSent << std::endl;
	}

	if (running) {
		receiveNext();
	}
}

void EthernetCommunication::close() {
	running = false;

	if (socket != NULL) {
		try {
			boost::system::error_code ec;
			socket->shutdown(boost::asio::ip::tcp::socket::shutdown_both, ec);
			socket->close();
		} catch (std::exception& e) {
			std::cout << "- EthernetCommunication close error: " << e.what() << std::endl;
		}
	}
}
