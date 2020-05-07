#include "GroupConfigManager.h"

namespace uni {
namespace master {

GroupConfigManager::GroupConfigManager(
  uni::async::AsyncQueue& async_queue,
  uni::net::Connections& slave_connections,
  uni::paxos::MultiPaxosHandler& multipaxos_handler,
  uni::paxos::PaxosLog& paxos_log)
  : _async_queue(async_queue),
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
  auto slave = std::vector<uni::net::EndpointId>();
  std::visit(overloaded {
    [&slave](Config config){ slave = config.slaves; },
    [&slave](NewConfig new_config){ slave = new_config.slaves; }
  }, _slave_group_config[group_id]);
  return slave;
}

boost::optional<uni::server::SlaveGroupId> GroupConfigManager::get_group_id(
  uni::net::EndpointId const& endpoint_id
) {
  for (auto const& [group_id, config_state] : _slave_group_config) {
    auto slave = std::vector<uni::net::EndpointId>();
    std::visit(overloaded {
      [&slave](Config config){ slave = config.slaves; },
      [&slave](NewConfig new_config){ slave = new_config.slaves; }
    }, config_state);
    auto const& it = std::find(slave.begin(), slave.end(), endpoint_id);
    if (it != slave.end()) {
      // The input endpoint_id is in the list of slaves.
      return group_id;
    }
  }
  return boost::none;
}

} // namespace master
} // namespace uni
