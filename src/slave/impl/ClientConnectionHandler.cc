#include "ClientConnectionHandler.h"

#include <memory>
#include <iostream>

#include <assert/assert.h>
#include <net/impl/ChannelImpl.h>

namespace uni {
namespace slave {

using boost::asio::ip::tcp;

ClientConnectionHandler::ClientConnectionHandler(
    uni::async::AsyncScheduler& scheduler,
    tcp::acceptor& acceptor,
    uni::net::ConnectionsIn& connections_in)
      : _scheduler(scheduler),
        _acceptor(acceptor),
        _connections_in(connections_in) {}

void ClientConnectionHandler::async_accept() {
  _acceptor.async_accept([this](const boost::system::error_code &ec, tcp::socket socket) {
    if (!ec) {
      _connections_in.add_channel(
        std::make_unique<uni::net::ChannelImpl>(std::move(socket)));
      async_accept();
    } else {
      LOG(uni::logging::Level::ERROR, "Error accepting client connection: " + ec.message())
    }
  });
}

} // slave
} // uni
