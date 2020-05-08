#include "SlaveConfigManager.h"

namespace uni {
namespace slave {

SlaveConfigManager::SlaveConfigManager(
  uni::async::AsyncQueue& async_queue,
  uni::net::Connections& master_connections,
  uni::net::Connections& slave_connections,
  uni::paxos::MultiPaxosHandler& multipaxos_handler,
  uni::paxos::PaxosLog& paxos_log,
  uni::net::EndpointId self_endpoint_id)
  : _async_queue(async_queue),
    _master_connections(master_connections),
    _slave_connections(slave_connections),
    _multipaxos_handler(multipaxos_handler),
    _paxos_log(paxos_log),
    _config(SlaveConfigManager::Config{{self_endpoint_id}, 0}) {}

std::vector<uni::net::EndpointId> SlaveConfigManager::config_endpoints() const {
  return _slave_connections.get_all_endpoints();
}

} // namespace slave
} // namespace uni
