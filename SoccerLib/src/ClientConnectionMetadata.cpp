#include "ClientConnectionMetadata.h"


ClientConnectionMetadata::ClientConnectionMetadata(websocketpp::connection_hdl hdl, std::string uri)
	: m_hdl(hdl)
	, m_status("Connecting")
	, m_uri(uri)
	, m_server("N/A")
{}

void ClientConnectionMetadata::on_open(client * c, websocketpp::connection_hdl hdl) {
	m_status = "Open";

	client::connection_ptr con = c->get_con_from_hdl(hdl);
	m_server = con->get_response_header("Server");
}

void ClientConnectionMetadata::on_fail(client * c, websocketpp::connection_hdl hdl) {
	m_status = "Failed";

	client::connection_ptr con = c->get_con_from_hdl(hdl);
	m_server = con->get_response_header("Server");
	m_error_reason = con->get_ec().message();
}

void ClientConnectionMetadata::on_close(client * c, websocketpp::connection_hdl hdl) {
	m_status = "Closed";
	client::connection_ptr con = c->get_con_from_hdl(hdl);
	std::stringstream s;
	s << "close code: " << con->get_remote_close_code() << " ("
		<< websocketpp::close::status::get_string(con->get_remote_close_code())
		<< "), close reason: " << con->get_remote_close_reason();
	m_error_reason = s.str();
}

void ClientConnectionMetadata::on_message(websocketpp::connection_hdl, client::message_ptr msg) {
	/*if (msg->get_opcode() == websocketpp::frame::opcode::text) {
		m_messages.push_back("<< " + msg->get_payload());
	}
	else {
		m_messages.push_back("<< " + websocketpp::utility::to_hex(msg->get_payload()));
	}
	*/
	m_messages.push(msg->get_payload());
	// std::cout << msg->get_payload() << std::endl;
}

websocketpp::connection_hdl ClientConnectionMetadata::get_hdl() const {
	return m_hdl;
}

std::string ClientConnectionMetadata::get_status() const {
	return m_status;
}

std::string ClientConnectionMetadata::dequeueMessage() {

	if (m_messages.size() == 0) {
		return "";
	}

	std::string message = m_messages.front();

	m_messages.pop();

	return message;
}

std::ostream & operator<< (std::ostream & out, ClientConnectionMetadata const & data) {
	out << "> URI: " << data.m_uri << "\n"
		<< "> Status: " << data.m_status << "\n"
		<< "> Remote Server: " << (data.m_server.empty() ? "None Specified" : data.m_server) << "\n"
		<< "> Error/close reason: " << (data.m_error_reason.empty() ? "N/A" : data.m_error_reason) << "\n";

	return out;
}