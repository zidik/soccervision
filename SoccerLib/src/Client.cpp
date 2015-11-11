#include "Client.h"


Client::Client() {
	m_endpoint.clear_access_channels(websocketpp::log::alevel::all);
	m_endpoint.clear_error_channels(websocketpp::log::elevel::all);

	m_endpoint.init_asio();
	m_endpoint.start_perpetual();

	m_thread = websocketpp::lib::make_shared<websocketpp::lib::thread>(&client::run, &m_endpoint);
}

Client::~Client() {
	m_endpoint.stop_perpetual();

	if (metadata_ptr && metadata_ptr->get_status() == "Open") {
		// Only close open connections
		std::cout << "> Closing client connection" << std::endl;

		websocketpp::lib::error_code ec;
		m_endpoint.close(metadata_ptr->get_hdl(), websocketpp::close::status::going_away, "", ec);
		if (ec) {
			std::cout << "> Error closing client connection" << ": "
				<< ec.message() << std::endl;
		}
	}

	m_thread->join();
}

void Client::connect(std::string const & uri) {
	websocketpp::lib::error_code ec;
	this->uri = uri;

	client::connection_ptr con = m_endpoint.get_connection(uri, ec);

	if (ec) {
		std::cout << "> Connect initialization error: " << ec.message() << std::endl;
		return;
	}

	metadata_ptr = websocketpp::lib::make_shared<ClientConnectionMetadata>(con->get_handle(), uri);

	con->set_open_handler(websocketpp::lib::bind(
		&ClientConnectionMetadata::on_open,
		metadata_ptr,
		&m_endpoint,
		websocketpp::lib::placeholders::_1
		));
	con->set_fail_handler(websocketpp::lib::bind(
		&ClientConnectionMetadata::on_fail,
		metadata_ptr,
		&m_endpoint,
		websocketpp::lib::placeholders::_1
		));
	con->set_close_handler(websocketpp::lib::bind(
		&ClientConnectionMetadata::on_close,
		metadata_ptr,
		&m_endpoint,
		websocketpp::lib::placeholders::_1
		));
	con->set_message_handler(websocketpp::lib::bind(
		&ClientConnectionMetadata::on_message,
		metadata_ptr,
		websocketpp::lib::placeholders::_1,
		websocketpp::lib::placeholders::_2
		));

	m_endpoint.connect(con);

}

void Client::close(websocketpp::close::status::value code, std::string reason) {
	websocketpp::lib::error_code ec;

	m_endpoint.close(metadata_ptr->get_hdl(), code, reason, ec);
	if (ec) {
		std::cout << "> Error initiating close: " << ec.message() << std::endl;
	}
}

void Client::send(std::string message) {
	websocketpp::lib::error_code ec;

	m_endpoint.send(metadata_ptr->get_hdl(), message, websocketpp::frame::opcode::text, ec);
	if (ec) {
		std::cout << "> Error sending message: " << ec.message() << std::endl;
		if (metadata_ptr->get_status() != "Connecting"){
			// reconnect if the socket was closed but a new message is sent
			connect(uri);
		}
		return;
	}

	//metadata_it->second->record_sent_message(message);
}
/*
ClientConnectionMetadata::ptr Client::get_metadata() const {
	if (metadata_ptr != NULL) {
		return metadata_ptr;
	}
	else {
		return ClientConnectionMetadata::ptr();
	}
}
*/
std::string Client::dequeueMessage() {

	return metadata_ptr->dequeueMessage();
}