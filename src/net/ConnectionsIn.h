#ifndef UNI_NET_INCONNECTIONS_H
#define UNI_NET_INCONNECTIONS_H

#include <memory>
#include <mutex>
#include <unordered_map>

#include <boost/optional.hpp>

#include <async/AsyncScheduler.h>
#include <common/common.h>
#include <net/Channel.h>
#include <net/endpoint_id.h>

namespace uni {
namespace net {

class ConnectionsIn {
 public:
  ConnectionsIn(uni::async::AsyncScheduler& scheduler);

  void add_channel(std::shared_ptr<uni::net::Channel> channel);

  // This method is primary used by the server thread to get the Channel object from it's endpoint_id
  // object. This is useful if we have to send data out on that channel.
  boost::optional<std::shared_ptr<uni::net::Channel>> get_channel(uni::net::endpoint_id endpoint_id);

 private:
  std::mutex _channel_lock;
  std::unordered_map<uni::net::endpoint_id, std::shared_ptr<uni::net::Channel>> _channels;
  uni::async::AsyncScheduler& _scheduler;
};

} // net
} // uni

#endif // UNI_NET_INCONNECTIONS_H
