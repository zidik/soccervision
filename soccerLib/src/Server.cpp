#include "Server.h"

#include <iostream>

Server::Server() : ws(NULL) {
	ws = new WebSocketServer();
	ws->addListener(this);

	port = 8000;
	clientCounter = 0;
}

Server::~Server() {
	std::cout << "! Closing web socket server.. ";

	close();
	join();

	std::cout << "done!" << std::endl;

	Client* client;

	for (ClientsIt it = clients.begin(); it != clients.end(); it++) {
		client = it->second;

		delete client;
	}

	clients.clear();

	Message* message;

	while (messages.size() > 0) {
		message = messages.front();
		messages.pop();

		delete message;
	}

	if (ws != NULL) delete ws; ws = NULL;
}

void Server::Message::respond(std::string response) {
	server->send(client->connection, response);
}

void Server::setPort(int port) {
	this->port = port;
}

void Server::broadcast(std::string message) {
	ws->broadcast(message);
}

void Server::close() {
	if (ws != NULL) {
		ws->stop();
	}
}

bool Server::gotMessages() {
	boost::mutex::scoped_lock lock(messagesMutex);

	return messages.size() > 0;
}

Server::Message* Server::dequeueMessage() {
	boost::mutex::scoped_lock lock(messagesMutex);
	
	if (messages.size() == 0) {
		return NULL;
	}

	Message* message = messages.front();

	messages.pop();

	return message;
}

Server::Client* Server::getClientByConnection(websocketpp::connection_hdl connection) {
	int id;
	Client* client;

	for (ClientsIt it = clients.begin(); it != clients.end(); it++) {
		id = it->first;
		client = it->second;

		if (client->connection.lock() == connection.lock()) {
			return client;
		}
	}

	return NULL;
}

void* Server::run() {
	std::cout << "! Starting web socket server on port " << port << std::endl;

	try {
		ws->listen(port);
	} catch (std::exception& e) {
		std::cout << "- Web socket server error: " << e.what() << std::endl;
	} catch (...) {
		std::cout << "- Web socket server error" << std::endl;
	}

	return NULL;
}

void Server::onSocketOpen(websocketpp::connection_hdl connection) {
	int id = clientCounter++;

	clients[id] = new Client(id, connection);

	std::cout << "! Server client #" << id << " connected" << std::endl;
}

void Server::onSocketClose(websocketpp::connection_hdl connection) {
	int id;
	Client* client;

	for (ClientsIt it = clients.begin(); it != clients.end(); it++) {
		id = it->first;
		client = it->second;

		if (client->connection.lock() == connection.lock()) {
			std::cout << "! Server client #" << id << " disconnected" << std::endl;

			clients.erase(it);

			return;
		}
	}

	std::cout << "- Unknown server client disconnected" << std::endl;
}

void Server::onSocketMessage(std::string message, websocketpp::connection_hdl connection, websocketpp::server<websocketpp::config::asio>::message_ptr msg) {
	Client* client = getClientByConnection(connection);

	if (client == NULL) {
		std::cout << "- Unknown client sent message: " << message << std::endl;

		return;
	}

	boost::mutex::scoped_lock lock(messagesMutex);
	messages.push(new Message(client, this, message));
	
	//std::cout << "! Server client #" << client->id << " sent message: " << message << std::endl;
}