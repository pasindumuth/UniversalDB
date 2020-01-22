#include "ConnectionsIn.h"

#include <assert/assert.h>

namespace uni {
namespace net {

using uni::async::AsyncScheduler;
using uni::net::Channel;

ConnectionsIn::ConnectionsIn(
    AsyncScheduler& scheduler)
    : _scheduler(scheduler) {}

void ConnectionsIn::add_channel(std::shared_ptr<Channel> channel) {
  auto endpoint_id = channel->endpoint_id();
  channel->set_receive_callback([endpoint_id, this](std::string message) {
    _scheduler.queue_message({endpoint_id, message});
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
  std::unique_lock<std::mutex> lock(_channel_lock);
  _channels.insert({endpoint_id, channel});
}

boost::optional<std::shared_ptr<Channel>> ConnectionsIn::get_channel(
    uni::net::endpoint_id endpoint_id) {
  std::unique_lock<std::mutex> lock(_channel_lock);
  auto it = _channels.find(endpoint_id);
  if (it != _channels.end()) {
    return boost::optional<std::shared_ptr<Channel>>(it->second);
  } else {
    return boost::optional<std::shared_ptr<Channel>>();
  }
}

} // net
} // uni
