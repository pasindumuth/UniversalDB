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

} // namespace master
} // namespace uni
