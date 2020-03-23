#include "SlaveConfigManager.h"

namespace uni {
namespace slave {

SlaveConfigManager::SlaveConfigManager(
  uni::async::AsyncQueue& async_queue,
  uni::net::Connections& master_connections,
  uni::net::Connections& connections,
  uni::paxos::MultiPaxosHandler& multipaxos_handler,
  uni::paxos::PaxosLog& paxos_log):
    _async_queue(async_queue),
    _master_connections(master_connections),
    _connections(connections),
    _multipaxos_handler(multipaxos_handler),
    _paxos_log(paxos_log) {}

std::vector<uni::net::EndpointId> SlaveConfigManager::config_endpoints() const {
  return _connections.get_all_endpoints();
}

} // namespace slave
} // namespace uni
