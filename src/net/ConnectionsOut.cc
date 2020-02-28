#include "ConnectionsOut.h"

#include <iostream>

#include <assert/assert.h>
#include <constants/constants.h>

namespace uni {
namespace net {

ConnectionsOut::ConnectionsOut(
    uni::constants::Constants const &constants)
      : _constants(constants) {}

void ConnectionsOut::add_channel(std::shared_ptr<uni::net::Channel> channel) {
  auto endpoint_id = channel->endpoint_id();
  channel->add_receive_callback([endpoint_id, this](std::string message) {
    UNIVERSAL_TERMINATE("A Channel in an OutConnections object should never receive data.");
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
  _channels.insert({endpoint_id, channel});
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
  uni::net::endpoint_id id(endpoint_id.ip_string, _constants.slave_port);
  std::unique_lock<std::mutex> lock(_channel_lock);
  auto it = _channels.find(id);
  if (it != _channels.end()) {
    auto channel = it->second;
    channel->queue_send(message);
  }
}

} // net
} // uni
