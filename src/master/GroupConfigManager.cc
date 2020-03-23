#include "GroupConfigManager.h"

namespace uni {
namespace master {

GroupConfigManager::GroupConfigManager(
  uni::async::AsyncQueue& async_queue,
  uni::net::Connections& connections,
  uni::net::Connections& slave_connections,
  uni::paxos::MultiPaxosHandler& multipaxos_handler,
  uni::paxos::PaxosLog& paxos_log):
    _async_queue(async_queue),
    _connections(connections),
    _slave_connections(slave_connections),
    _multipaxos_handler(multipaxos_handler),
    _paxos_log(paxos_log) {}

void GroupConfigManager::set_first_config(
  uni::server::SlaveGroupId group_id,
  std::vector<uni::net::EndpointId>& slave_endpoints
) {
  _slave_group_config.insert({group_id, Config{slave_endpoints, 0}});
}

template<class... Ts> struct overloaded : Ts... { using Ts::operator()...; };
template<class... Ts> overloaded(Ts...) -> overloaded<Ts...>;

std::vector<uni::net::EndpointId> GroupConfigManager::get_endpoints(
  uni::server::SlaveGroupId const& group_id
) {
  std::vector<uni::net::EndpointId> slave;
  std::visit(overloaded {
    [&slave](Config config){ slave = config.slaves; },
    [&slave](NewConfig new_config){ slave = new_config.slaves; }
  }, _slave_group_config[group_id]);
  return slave;
}

} // namespace master
} // namespace uni
