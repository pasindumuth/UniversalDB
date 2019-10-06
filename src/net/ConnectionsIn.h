#ifndef UNI_NET_INCONNECTIONS_H
#define UNI_NET_INCONNECTIONS_H

#include <memory>
#include <unordered_map>

#include <async/AsyncScheduler.h>
#include <net/Channel.h>
#include <net/endpoint_id.h>

namespace uni {
namespace net {

class ConnectionsIn {
 public:
  ConnectionsIn(uni::async::AsyncScheduler& scheduler);

  void add_channel(std::shared_ptr<uni::net::Channel> channel);

 private:
  std::unordered_map<uni::net::endpoint_id, std::shared_ptr<uni::net::Channel>> _channels;
  uni::async::AsyncScheduler& _scheduler;
};

} // net
} // uni

#endif // UNI_NET_INCONNECTIONS_H
