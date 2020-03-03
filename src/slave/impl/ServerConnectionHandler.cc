#include "ServerConnectionHandler.h"

#include <net/impl/ChannelImpl.h>

namespace uni {
namespace slave {

using boost::asio::ip::tcp;

ServerConnectionHandler::ServerConnectionHandler(
    uni::constants::Constants const& constants,
    uni::net::ConnectionsOut& connections_out,
    tcp::acceptor& acceptor,
    boost::asio::io_context& io_context)
    : _constants(constants),
      _connections_out(connections_out),
      _acceptor(acceptor),
      _io_context(io_context) {}

void ServerConnectionHandler::async_accept() {
  _acceptor.async_accept([this](const boost::system::error_code &ec, tcp::socket socket) {
    if (!ec) {
      LOG(uni::logging::Level::TRACE2, "Received a connection from " + socket.remote_endpoint().address().to_string())
      _connections_out.add_channel(
        std::make_unique<uni::net::ChannelImpl>(std::move(socket)));
      async_accept();
    } else {
      LOG(uni::logging::Level::ERROR, "Error receiving a connection: " + ec.message())
    }
  });
}

} // slave
} // uni
