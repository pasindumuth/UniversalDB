#include "ClientConnectionHandler.h"

#include <iostream>

#include <assert/assert.h>
#include <logging/log.h>
#include <net/impl/ChannelImpl.h>

namespace uni {
namespace slave {

using uni::async::AsyncScheduler;
using uni::net::Channel;
using boost::asio::ip::tcp;

ClientConnectionHandler::ClientConnectionHandler(
    std::shared_ptr<AsyncScheduler> scheduler,
    std::shared_ptr<tcp::acceptor> acceptor)
      : _scheduler(scheduler),
        _acceptor(acceptor) {}

void ClientConnectionHandler::async_accept() {
  _acceptor->async_accept([this](const boost::system::error_code &ec, tcp::socket socket) {
    if (!ec) {
      std::unique_lock<std::mutex> lock(_channel_lock);
      auto channel = std::make_shared<uni::net::ChannelImpl>(std::move(socket));
      auto endpoint_id = channel->endpoint_id();
      channel->set_recieve_callback([endpoint_id, this](std::string message) {
        _scheduler->schedule_async({endpoint_id, message});
      });
      channel->set_close_callback([endpoint_id, this]() {
        std::unique_lock<std::mutex> lock(_channel_lock);
        auto it = _channels.find(endpoint_id);
        if (it != _channels.end()) {
          _channels.erase(it);
        } else {
          UNIVERSAL_TERMINATE("Channel cannot/should not be deleted whenever this callback is run");
        }
      });
      channel->start_listening();
      _channels.insert({ endpoint_id, channel });
      async_accept();
    } else {
      LOG(uni::logging::Level::ERROR, ec.message())
    }
  });
}

boost::optional<std::shared_ptr<Channel>> ClientConnectionHandler::get_channel(
    uni::net::endpoint_id endpoint_id) {
  std::unique_lock<std::mutex> lock(_channel_lock);
  auto it = _channels.find(endpoint_id);
  if (it != _channels.end()) {
    return boost::optional<std::shared_ptr<Channel>>(it->second);
  } else {
    return boost::optional<std::shared_ptr<Channel>>();
  }
}

} // slave
} // uni
