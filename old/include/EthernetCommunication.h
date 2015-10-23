#ifndef ETHERNET_COMMUNICATION_H
#define ETHERNET_COMMUNICATION_H

#include "AbstractCommunication.h"

#include <boost/thread/mutex.hpp>
#include <boost/asio.hpp>
#include <boost/array.hpp>
#include <string>
#include <vector>
#include <stack>
#include <queue>

using boost::asio::ip::udp;

class EthernetCommunication : public AbstractCommunication {

public:
	
	EthernetCommunication(std::string host = "127.0.0.1", int port = 8042);
	~EthernetCommunication();

	void send(std::string message);
	bool gotMessages();
	std::string dequeueMessage();
	void close();

private:
	void* run();
	void onReceive(const boost::system::error_code& error, size_t bytesReceived);
	void onSend(const boost::system::error_code& error, size_t bytesSent);
	void receiveNext();

	std::string host;
	int port;
	char receiveBuffer[MAX_SIZE];
	boost::asio::mutable_buffers_1 receiveBuffer2;
	//boost::array<char, 1024> receiveBuffer;
	char requestBuffer[MAX_SIZE];
	boost::asio::io_service ioService;
	udp::socket* socket;
	udp::endpoint endpoint;
	boost::asio::ip::udp::endpoint remoteEndpoint;
	Messages messages;
	Messages queuedMessages;
	bool running;
	mutable boost::mutex messagesMutex;
};

#endif // ETHERNET_COMMUNICATION_H