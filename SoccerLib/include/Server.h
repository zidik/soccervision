#ifndef SERVER_H
#define SERVER_H

#include "Thread.h"
#include "WebSocketServer.h"

#include <boost/thread/mutex.hpp>
#include <string>
#include <queue>
#include <map>

class Server : public Thread, WebSocketServer::Listener {

public:
	struct Client {
		Client(int id, websocketpp::connection_hdl connection) : id(id), connection(connection) {}

		int id;
		websocketpp::connection_hdl connection;
	};

	struct Message {
		Message(Client* client, Server* server, std::string content) : client(client), server(server), content(content) {}
		void respond(std::string response);

		Client* client;
		Server* server;
		std::string content;
	};

	typedef std::queue<Message*> Messages;
	typedef std::map<int, Client*> Clients;
	typedef Clients::iterator ClientsIt;

	Server();
	~Server();

	void setPort(int port);
	void broadcast(std::string message);
	void send(websocketpp::connection_hdl connection, std::string message) { ws->send(connection, message); }
	void close();
	bool gotMessages();
	Message* dequeueMessage();
	Client* getClientByConnection(websocketpp::connection_hdl connection);

private:
	void* run();
	void onSocketOpen(websocketpp::connection_hdl connection);
	void onSocketClose(websocketpp::connection_hdl connection);
	void onSocketMessage(std::string message, websocketpp::connection_hdl connection, websocketpp::server<websocketpp::config::asio>::message_ptr msg);

	WebSocketServer* ws;
	int port;
	int clientCounter;
	Clients clients;
	Messages messages;
	mutable boost::mutex messagesMutex;
};

#endif