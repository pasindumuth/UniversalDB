#include "ConnectionsOut.h"

#include <iostream>

#include <assert/assert.h>
#include <constants/constants.h>

namespace uni {
namespace net {

using uni::constants::Constants;
using uni::net::Channel;

ConnectionsOut::ConnectionsOut(
    uni::constants::Constants const &constants)
      : _constants(constants) {}

void ConnectionsOut::add_channel(std::shared_ptr<Channel> channel) {
  std::unique_lock<std::mutex> lock(_channel_lock);
  auto endpoint_id = channel->endpoint_id();
  channel->set_receive_callback([endpoint_id, this](std::string message) {
    UNIVERSAL_TERMINATE("A Channel in an OutConnections object should never receive data.");
    return true;
  });
  channel->set_close_callback([endpoint_id, this]() {
    std::unique_lock<std::mutex> lock(_channel_lock);
    auto it = _channels.find(endpoint_id);
    if (it != _channels.end()) {
      _channels.erase(it);
    } else {
      UNIVERSAL_TERMINATE("A Channel cannot/should not be deleted whenever this callback is run.");
    }
  });
  channel->start_listening();
  _channels.insert({endpoint_id, channel});
}

void ConnectionsOut::broadcast(std::string message) {
  for (auto const& channel : _channels) {
    channel.second->queue_send(message);
  }
}

bool ConnectionsOut::has(uni::net::endpoint_id endpoint_id) {
  return _channels.find(endpoint_id) != _channels.end();
}

void ConnectionsOut::send(uni::net::endpoint_id const& endpoint_id, std::string message) {
  uni::net::endpoint_id id(endpoint_id.ip_string, _constants.slave_port);
  auto it = _channels.find(id);
  if (it != _channels.end()) {
    auto channel = it->second;
    channel->queue_send(message);
  }
}

} // net
} // uni
