#ifndef CLIENTCONNECTIONMETADATA_H
#define CLIENTCONNECTIONMETADATA_H

#include <websocketpp/config/asio_no_tls_client.hpp>
#include <websocketpp/client.hpp>

#include <websocketpp/common/thread.hpp>
#include <websocketpp/common/memory.hpp>


class ClientConnectionMetadata {
public:
	typedef websocketpp::lib::shared_ptr<ClientConnectionMetadata> ptr;
	typedef websocketpp::client<websocketpp::config::asio_client> client;

	ClientConnectionMetadata(websocketpp::connection_hdl hdl, std::string uri);

	void on_open(client * c, websocketpp::connection_hdl hdl);
	void on_fail(client * c, websocketpp::connection_hdl hdl);
	void on_close(client * c, websocketpp::connection_hdl hdl);
	void on_message(websocketpp::connection_hdl, client::message_ptr msg);
	websocketpp::connection_hdl get_hdl() const;
	std::string get_status() const;
	std::string ClientConnectionMetadata::dequeueMessage();

	friend std::ostream & operator<< (std::ostream & out, ClientConnectionMetadata const & data);
private:
	websocketpp::connection_hdl m_hdl;
	std::string m_status;
	std::string m_uri;
	std::string m_server;
	std::string m_error_reason;
	std::queue<std::string> m_messages;
};

#endif