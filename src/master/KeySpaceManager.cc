#include "KeySpaceManager.h"

namespace uni {
namespace master {

KeySpaceManager::KeySpaceManager(
  uni::async::AsyncQueue& async_queue,
  uni::master::GroupConfigManager& config_manager,
  uni::net::Connections& slave_connections,
  uni::paxos::MultiPaxosHandler& multipaxos_handler,
  uni::paxos::PaxosLog& paxos_log):
    _async_queue(async_queue),
    _config_manager(config_manager),
    _slave_connections(slave_connections),
    _multipaxos_handler(multipaxos_handler),
    _paxos_log(paxos_log) {}

void KeySpaceManager::set_first_config(uni::server::SlaveGroupId group_id) {
  _slave_group_ranges.insert({group_id, KeySpace{{}, 0}});
}

} // namespace master
} // namespace uni
