#ifndef SERVER_H
#define SERVER_H

#include <cstring>
#include <map>
#include <memory>
#include <restbed>
#include <stdio.h>
#include <string>
#include <system_error>
#include <utility>

using namespace restbed;

namespace game
{

class Server
{
  public:
    Server();
    ~Server();

    void Run();
    template <class TMessage> void SendMessage(const TMessage& message);
    template <class TMessage>
    void SendMessage(const std::shared_ptr<WebSocket> socket,
                     const TMessage& message);

  private:
    std::multimap<std::string, std::string>
    build_websocket_handshake_response_headers(
        const std::shared_ptr<const restbed::Request>& request);
    void ping_handler(void);
    void get_method_handler(const std::shared_ptr<Session> session);
    void message_handler(const std::shared_ptr<WebSocket> source,
                         const std::shared_ptr<WebSocketMessage> message);
    void error_handler(const std::shared_ptr<WebSocket> socket,
                       const std::error_code error);
    void close_handler(const std::shared_ptr<WebSocket> socket);

  private:
    std::shared_ptr<restbed::Service> service = nullptr;
    std::map<std::string, std::shared_ptr<restbed::WebSocket>> sockets = {};

    std::function<void(const std::shared_ptr<WebSocket>)> f_close_handler;
    std::function<void(const std::shared_ptr<WebSocket>, const std::error_code)>
        f_error_handler;
    std::function<void(const std::shared_ptr<WebSocket>,
                       const std::shared_ptr<WebSocketMessage>)>
        f_message_handler;

    std::thread m_serviceThread;
};

template <class TMessage> void Server::SendMessage(const TMessage& message)
{
    for(auto socket_pair : this->sockets) {
	SendMessage(socket_pair.second, message);
    }
}

template <class TMessage>
void Server::SendMessage(const std::shared_ptr<WebSocket> socket,
                         const TMessage& message)
{
    auto msg = message.SerializeAsString();
    socket->send(msg);
    fprintf(stderr, "Sending message\n");
}

} // namespace game

#endif // SERVER_H
