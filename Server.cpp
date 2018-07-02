#include "Encoding.h"
#include "Server.h"
#include "proto/message.pb.h"
#include <chrono>
#include <cstring>
#include <functional>
#include <iostream>
#include <openssl/buffer.h>
#include <openssl/evp.h>
#include <openssl/hmac.h>
#include <openssl/sha.h>
#include <string>
#include <system_error>
#include <thread>
#include <utility>

using namespace std;
using namespace restbed;
using namespace placeholders;
using namespace chrono;

namespace game
{
Server::Server()
{
    f_close_handler = std::bind(&Server::close_handler, this, _1);
    f_error_handler = std::bind(&Server::error_handler, this, _1, _2);
    f_message_handler = std::bind(&Server::message_handler, this, _1, _2);

    service = make_shared<Service>();
}

Server::~Server()
{
    service->stop();
    m_serviceThread.join();
}

void Server::Run()
{
    auto thread_task = [&]() {
	std::cout << "creating service" << std::endl;
	auto resource = make_shared<Resource>();
	resource->set_path("/chat");

	resource->set_method_handler(
	    "GET", std::bind(&Server::get_method_handler, this, _1));

	service->publish(resource);
	service->schedule(std::bind(&Server::ping_handler, this),
	                  milliseconds(5000));

	auto settings = make_shared<Settings>();
	settings->set_port(8082);
	service->start(settings);
    };

    m_serviceThread = std::thread(thread_task);
}

multimap<string, string> Server::build_websocket_handshake_response_headers(
    const shared_ptr<const Request>& request)
{
    auto key = request->get_header("Sec-WebSocket-Key");
    key.append("258EAFA5-E914-47DA-95CA-C5AB0DC85B11");

    restbed::Byte hash[SHA_DIGEST_LENGTH];
    SHA1(reinterpret_cast<const unsigned char*>(key.data()), key.length(),
         hash);

    std::multimap<string, string> headers;
    headers.insert(make_pair("Upgrade", "websocket"));
    headers.insert(make_pair("Connection", "Upgrade"));
    headers.insert(make_pair("Sec-WebSocket-Accept",
                             enc::base64_encode(hash, SHA_DIGEST_LENGTH)));

    return headers;
}

void Server::ping_handler(void)
{
    for(auto entry : sockets) {
	auto key = entry.first;
	auto socket = entry.second;

	if(socket->is_open()) {
	    socket->send(restbed::WebSocketMessage::PING_FRAME);
	} else {
	    socket->close();
	}
    }
}

void Server::close_handler(const shared_ptr<WebSocket> socket)
{
    if(socket->is_open()) {
	auto response = make_shared<WebSocketMessage>(
	    WebSocketMessage::CONNECTION_CLOSE_FRAME, Bytes({10, 00}));
	socket->send(response);
    }

    const auto key = socket->get_key();
    sockets.erase(key);

    fprintf(stderr, "Closed connection to %s.\n", key.data());
}

void Server::error_handler(const shared_ptr<WebSocket> socket,
                           const error_code error)
{
    const auto key = socket->get_key();
    fprintf(stderr, "WebSocket Errored '%s' for %s.\n", error.message().data(),
            key.data());
}

void Server::message_handler(const shared_ptr<WebSocket> source,
                             const shared_ptr<WebSocketMessage> message)
{
    const auto opcode = message->get_opcode();

    if(opcode == WebSocketMessage::PING_FRAME) {
	auto response = make_shared<WebSocketMessage>(
	    WebSocketMessage::PONG_FRAME, message->get_data());
	source->send(response);
    } else if(opcode == WebSocketMessage::PONG_FRAME) {
	return;
    } else if(opcode == WebSocketMessage::CONNECTION_CLOSE_FRAME) {
	source->close();
    } else if(opcode == WebSocketMessage::BINARY_FRAME) {
	// We don't support binary data.
	auto response = make_shared<WebSocketMessage>(
	    WebSocketMessage::CONNECTION_CLOSE_FRAME, Bytes({10, 03}));
	source->send(response);
    } else if(opcode == WebSocketMessage::TEXT_FRAME) {
	auto response = make_shared<WebSocketMessage>(*message);
	response->set_mask(0);

	for(auto socket : sockets) {
	    auto destination = socket.second;
	    destination->send(response);
	}

	const auto key = source->get_key();
	const auto data = String::format(
	    "Received message '%.*s' from %s\n", message->get_data().size(),
	    message->get_data().data(), key.data());
	fprintf(stderr, "%s", data.data());
    }
}

void Server::get_method_handler(const shared_ptr<Session> session)
{
    const auto request = session->get_request();
    const auto connection_header =
        request->get_header("connection", String::lowercase);

    if(connection_header.find("upgrade") not_eq string::npos) {
	if(request->get_header("upgrade", String::lowercase) == "websocket") {
	    const auto headers =
	        build_websocket_handshake_response_headers(request);

	    session->upgrade(
	        SWITCHING_PROTOCOLS, headers,
	        [&](const shared_ptr<WebSocket> socket) {
		    if(socket->is_open()) {
		        socket->set_close_handler(f_close_handler);
		        socket->set_error_handler(f_error_handler);
		        socket->set_message_handler(f_message_handler);

                const auto key = socket->get_key( );
                sockets.insert( make_pair( key, socket ) );

		        GreetMessage greetMessage;
		        greetMessage.set_text("Hello from GreetMessage, your socket key: " + key);
		        SendMessage(socket, greetMessage);
		    } else {
		        fprintf(stderr,
		                "WebSocket Negotiation Failed: Client closed "
		                "connection.\n");
		    }
	        });

	    return;
	}
    }

    session->close(BAD_REQUEST);
}
} // namespace game