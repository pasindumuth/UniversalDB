#include "ConnectionsOut.h"

#include <assert/assert.h>

namespace uni {
namespace net {

ConnectionsOut::ConnectionsOut(
    uni::async::AsyncScheduler& scheduler)
    : _scheduler(scheduler) {}

void ConnectionsOut::add_channel(std::unique_ptr<uni::net::Channel>&& channel) {
  auto endpoint_id = channel->endpoint_id();
  channel->add_receive_callback([endpoint_id, this](std::string message) {
    _scheduler.queue_message({endpoint_id, message});
    return true;
  });
  channel->add_close_callback([endpoint_id, this]() {
    std::unique_lock<std::mutex> lock(_channel_lock);
    auto it = _channels.find(endpoint_id);
    if (it != _channels.end()) {
      _channels.erase(it);
    } else {
      UNIVERSAL_TERMINATE("A Channel cannot/should not be deleted whenever this callback is run.");
    }
  });
  channel->start_listening();
  std::unique_lock<std::mutex> lock(_channel_lock);
  _channels.insert({endpoint_id, std::move(channel)});
}

boost::optional<uni::net::Channel&> ConnectionsOut::get_channel(
    uni::net::endpoint_id endpoint_id) {
  std::unique_lock<std::mutex> lock(_channel_lock);
  auto it = _channels.find(endpoint_id);
  if (it != _channels.end()) {
    return boost::optional<uni::net::Channel&>(*it->second);
  } else {
    return boost::optional<uni::net::Channel&>();
  }
}

void ConnectionsOut::broadcast(std::string message) {
  std::unique_lock<std::mutex> lock(_channel_lock);
  for (auto const& channel : _channels) {
    channel.second->queue_send(message);
  }
}

bool ConnectionsOut::has(uni::net::endpoint_id endpoint_id) {
  std::unique_lock<std::mutex> lock(_channel_lock);
  return _channels.find(endpoint_id) != _channels.end();
}

void ConnectionsOut::send(uni::net::endpoint_id const& endpoint_id, std::string message) {
  auto id = uni::net::endpoint_id(endpoint_id.ip_string, 0); // Of course, make it such that we don't need to do this.
  std::unique_lock<std::mutex> lock(_channel_lock);
  auto it = _channels.find(id);
  if (it != _channels.end()) {
    auto& channel = it->second;
    channel->queue_send(message);
  }
}

} // net
} // uni
