#include "WebSocketServer.h"

WebSocketServer::WebSocketServer() : server(NULL) {

}

WebSocketServer::~WebSocketServer() {
	stop();

	if (server != NULL) delete server; server = NULL;
}

void WebSocketServer::listen(int port) {
	std::cout << "! Starting web-socket server" << std::endl;

	server = new Server();

	try {
        /*server->set_access_channels(websocketpp::log::alevel::all);
        server->clear_access_channels(websocketpp::log::alevel::frame_payload);*/
        server->clear_access_channels(websocketpp::log::alevel::all);
        server->init_asio();

        server->set_open_handler(websocketpp::lib::bind(&WebSocketServer::onOpen, this, websocketpp::lib::placeholders::_1));
        server->set_close_handler(websocketpp::lib::bind(&WebSocketServer::onClose, this, websocketpp::lib::placeholders::_1));
        server->set_message_handler(websocketpp::lib::bind(&WebSocketServer::onMessage, this, websocketpp::lib::placeholders::_1, websocketpp::lib::placeholders::_2));
		
		// TODO How to add host?
        server->listen(port);
        server->start_accept();
        server->run();
    } catch (const std::exception & e) {
        std::cout << "- WebSocket error: " << e.what() << std::endl;
    } catch (websocketpp::lib::error_code e) {
		std::cout << "- WebSocket error: " << e.message() << std::endl;
    } catch (...) {
        std::cout << "other exception" << std::endl;
    }

	std::cout << "! Web-socket server killed" << std::endl;
}

void WebSocketServer::stop() {
	if (server == NULL) {
		return;
	}

	server->stop();
}

void WebSocketServer::addListener(Listener* listener) {
	listeners.push_back(listener);
}

void WebSocketServer::broadcast(std::string message) {
	for (ConnectionsIt it = connections.begin(); it != connections.end(); it++) {
		send(*it, message);
	}
}

void WebSocketServer::send(websocketpp::connection_hdl connection, std::string message) {
	try {
		server->send(connection, message, websocketpp::frame::opcode::TEXT);
	} catch (...) {
		std::cout << "! Sending server message '" << message << "' failed" << std::endl;
	}
}

void WebSocketServer::onOpen(websocketpp::connection_hdl connection) {
	connections.insert(connection);

	for (ListenersIt it = listeners.begin(); it != listeners.end(); it++) {
		(*it)->onSocketOpen(connection);
	}
}

void WebSocketServer::onClose(websocketpp::connection_hdl connection) {
	connections.erase(connection);

	for (ListenersIt it = listeners.begin(); it != listeners.end(); it++) {
		(*it)->onSocketClose(connection);
	}
}

void WebSocketServer::onMessage(websocketpp::connection_hdl connection, websocketpp::server<websocketpp::config::asio>::message_ptr msg) {
	for (ListenersIt it = listeners.begin(); it != listeners.end(); it++) {
		(*it)->onSocketMessage(msg->get_payload(), connection, msg);
	}
}