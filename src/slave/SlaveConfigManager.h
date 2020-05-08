#ifndef UNI_SLAVE_SLAVECONFIGMANAGER_H
#define UNI_SLAVE_SLAVECONFIGMANAGER_H

#include <variant>
#include <vector>

#include <async/AsyncQueue.h>
#include <common/common.h>
#include <net/EndpointId.h>
#include <paxos/MultiPaxosHandler.h>
#include <paxos/PaxosLog.h>

namespace uni {
namespace slave {

class SlaveConfigManager {
 public:
  SlaveConfigManager(
    uni::async::AsyncQueue& async_queue,
    uni::net::Connections& master_connections,
    uni::net::Connections& slave_connections,
    uni::paxos::MultiPaxosHandler& multipaxos_handler,
    uni::paxos::PaxosLog& paxos_log,
    uni::net::EndpointId self_endpoint_id);

  std::vector<uni::net::EndpointId> config_endpoints() const;

 private:
  uni::async::AsyncQueue& _async_queue;
  uni::net::Connections& _master_connections;
  uni::net::Connections& _slave_connections;
  uni::paxos::MultiPaxosHandler& _multipaxos_handler;
  uni::paxos::PaxosLog& _paxos_log;

  struct Config {
    std::vector<uni::net::EndpointId> slaves;
    uint32_t generation;
  };
  
  std::variant<
    Config
  > _config;
};

} // namespace slave
} // namespace uni


#endif // UNI_SLAVE_SLAVECONFIGMANAGER_H
