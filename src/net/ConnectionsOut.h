#ifndef UNI_NET_OUTCONNECTIONS_H
#define UNI_NET_OUTCONNECTIONS_H

#include <memory>
#include <mutex>
#include <string>
#include <unordered_map>

#include <constants/constants.h>
#include <net/Channel.h>
#include <net/endpoint_id.h>

namespace uni {
namespace net {

class ConnectionsOut {
 public:
  ConnectionsOut(uni::constants::Constants const& constants);

  void add_channel(std::shared_ptr<uni::net::Channel> channel);

  void broadcast(std::string message);

  bool has(uni::net::endpoint_id endpoint_id);

  // This method is primary used by the server thread to get the Channel object from it's endpoint_id
  // object. This is useful if we have to send data out on that channel.
  void send(uni::net::endpoint_id const& endpoint_id, std::string message);

 private:
  uni::constants::Constants const& _constants;
  std::unordered_map<uni::net::endpoint_id, std::shared_ptr<uni::net::Channel>> _channels;
  std::mutex _channel_lock;
};

} // net
} // uni

#endif // UNI_NET_OUTCONNECTIONS_H
