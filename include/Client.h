#ifndef CLIENT_H
#define CLIENT_H
/*
#include <websocketpp/config/asio_no_tls_client.hpp>
#include <websocketpp/client.hpp>

#include <websocketpp/common/thread.hpp>
#include <websocketpp/common/memory.hpp>
*/
//#include <cstdlib>
//#include <iostream>
//#include <map>
//#include <string>
//#include <sstream>

#include "ClientConnectionMetadata.h"

typedef websocketpp::client<websocketpp::config::asio_client> client;

class Client {
public:
	Client();

	~Client();

	void connect(std::string const & uri);

	void close(websocketpp::close::status::value code, std::string reason);

	void send(std::string message);

	//ClientConnectionMetadata::ptr get_metadata() const;

	std::string Client::dequeueMessage();
private:
	std::string uri;
	client m_endpoint;
	websocketpp::lib::shared_ptr<websocketpp::lib::thread> m_thread;

	ClientConnectionMetadata::ptr metadata_ptr;
};


#endif