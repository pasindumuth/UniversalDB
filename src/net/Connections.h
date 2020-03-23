#ifndef UNI_NET_CONNECTIONS_H
#define UNI_NET_CONNECTIONS_H

#include <memory>
#include <mutex>
#include <unordered_map>

#include <boost/optional.hpp>

#include <async/AsyncScheduler.h>
#include <common/common.h>
#include <net/Channel.h>
#include <net/EndpointId.h>

namespace uni {
namespace net {

class Connections {
 public:
  Connections(uni::async::AsyncScheduler& scheduler);

  void add_channel(std::unique_ptr<uni::net::Channel>&& channel);

  // This method is primary used by the server thread to get the Channel object from it's endpoint_id
  // object. This is useful if we have to send data out on that channel.
  boost::optional<uni::net::Channel&> get_channel(uni::net::EndpointId endpoint_id);

  // Sends the message to all endpoints passed into this function. If the endpoint doesn't
  // exists in _channels, then the endpoint is ignored (no errors are thrown).
  void broadcast(std::vector<uni::net::EndpointId> endpoints, std::string message);

  std::vector<uni::net::EndpointId> get_all_endpoints() const;

  bool has(uni::net::EndpointId endpoint_id);

  // This method is primary used by the server thread to get the Channel object from it's endpoint_id
  // object. This is useful if we have to send data out on that channel.
  void send(uni::net::EndpointId const& endpoint_id, std::string message);

 private:
  std::mutex _channel_lock;
  std::unordered_map<uni::net::EndpointId, std::unique_ptr<uni::net::Channel>> _channels;
  uni::async::AsyncScheduler& _scheduler;
};

} // net
} // uni

#endif // UNI_NET_CONNECTIONS_H
