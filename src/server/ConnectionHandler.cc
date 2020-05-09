#include "ConnectionHandler.h"

#include <net/impl/ChannelImpl.h>

namespace uni {
namespace server {

using boost::asio::ip::tcp;

ConnectionHandler::ConnectionHandler(
  uni::net::Connections& connections,
  tcp::acceptor& acceptor)
  : _connections(connections),
    _acceptor(acceptor) {}

void ConnectionHandler::async_accept() {
  _acceptor.async_accept([this](const boost::system::error_code &ec, tcp::socket socket) {
    if (!ec) {
      LOG(uni::logging::Level::TRACE2, "Received a connection from " + socket.remote_endpoint().address().to_string())
      _connections.add_channel(
        std::make_unique<uni::net::ChannelImpl>(std::move(socket)));
      async_accept();
    } else {
      LOG(uni::logging::Level::ERROR, "Error receiving a connection: " + ec.message())
    }
  });
}

} // namespace server
} // namespace uni
