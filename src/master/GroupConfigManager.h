#ifndef UNI_MASTER_GROUPCONFIGMANAGER_H
#define UNI_MASTER_GROUPCONFIGMANAGER_H

#include <unordered_map>
#include <variant>
#include <vector>

#include <common/common.h>

#include <async/AsyncQueue.h>
#include <server/SlaveGroupId.h>
#include <net/Connections.h>
#include <net/endpoint_id.h>
#include <paxos/MultiPaxosHandler.h>

namespace uni {
namespace master {

class GroupConfigManager {
 public:
  GroupConfigManager(
    uni::async::AsyncQueue& async_queue,
    uni::net::Connections& connections,
    uni::net::Connections& slave_connections,
    uni::paxos::MultiPaxosHandler& multipaxos_handler,
    uni::paxos::PaxosLog& paxos_log);

 private:
  uni::async::AsyncQueue& _async_queue;
  uni::net::Connections& _connections;
  uni::net::Connections& _slave_connections;
  uni::paxos::MultiPaxosHandler& _multipaxos_handler;
  uni::paxos::PaxosLog& _paxos_log;

  struct Config {
    std::vector<uni::net::endpoint_id> slaves;
    uint32_t generation;
  };

  struct NewConfig {
    std::vector<uni::net::endpoint_id> slaves;
    std::vector<uni::net::endpoint_id> new_slaves;
    uint32_t generation;
  };

  std::unordered_map<uni::server::SlaveGroupId, std::variant<Config, NewConfig>> _slave_group_config;
  std::vector<uni::net::endpoint_id> _spare_nodes;
};

} // namespace master
} // namespace uni


#endif // UNI_MASTER_GROUPCONFIGMANAGER_H
