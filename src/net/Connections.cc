#include "Connections.h"

#include <assert/assert.h>

namespace uni {
namespace net {

Connections::Connections(
  uni::async::AsyncScheduler& scheduler)
  : _scheduler(scheduler) {}

void Connections::add_channel(std::unique_ptr<uni::net::Channel>&& channel) {
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

boost::optional<uni::net::Channel&> Connections::get_channel(
    uni::net::EndpointId endpoint_id) {
  std::unique_lock<std::mutex> lock(_channel_lock);
  auto it = _channels.find(endpoint_id);
  if (it != _channels.end()) {
    return boost::optional<uni::net::Channel&>(*it->second);
  } else {
    return boost::optional<uni::net::Channel&>();
  }
}

void Connections::broadcast(std::vector<uni::net::EndpointId> endpoints, std::string message) {
  std::unique_lock<std::mutex> lock(_channel_lock);
  for (auto const& endpoint : endpoints) {
    auto it = _channels.find(endpoint);
    if (it != _channels.end()) {
      it->second->queue_send(message);
    }
  }
}

std::vector<uni::net::EndpointId> Connections::get_all_endpoints() const {
  auto endpoints = std::vector<EndpointId>();
  for (auto const& [key, value] : _channels) {
    endpoints.push_back(key);
  }
  return endpoints;
}

bool Connections::has(uni::net::EndpointId endpoint_id) {
  std::unique_lock<std::mutex> lock(_channel_lock);
  return _channels.find(endpoint_id) != _channels.end();
}

void Connections::send(uni::net::EndpointId const& endpoint_id, std::string message) {
  std::unique_lock<std::mutex> lock(_channel_lock);
  auto it = _channels.find(endpoint_id);
  if (it != _channels.end()) {
    auto& channel = it->second;
    channel->queue_send(message);
  }
}

} // namespace net
} // namespace uni
