#ifndef CLIENT_H
#define CLIENT_H

#include "ClientConnectionMetadata.h"

typedef websocketpp::client<websocketpp::config::asio_client> client;

class Client {
public:
	Client();

	~Client();

	void connect();

	void close(websocketpp::close::status::value code, std::string reason);

	void send(std::string message);

	//ClientConnectionMetadata::ptr get_metadata() const;

	void set_uri(std::string uri);

	std::string Client::dequeueMessage();
private:
	std::string uri;
	client m_endpoint;
	websocketpp::lib::shared_ptr<websocketpp::lib::thread> m_thread;

	ClientConnectionMetadata::ptr metadata_ptr;
};


#endif