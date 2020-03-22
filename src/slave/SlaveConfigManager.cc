#include "SlaveConfigManager.h"

namespace uni {
namespace slave {

SlaveConfigManager::SlaveConfigManager(
  uni::async::AsyncQueue& async_queue,
  uni::paxos::MultiPaxosHandler& multipaxos_handler,
  uni::paxos::PaxosLog& paxos_log,
  std::vector<uni::net::EndpointId>& config_endpoints):
    _async_queue(async_queue),
    _multipaxos_handler(multipaxos_handler),
    _paxos_log(paxos_log),
    _config_endpoints(config_endpoints) {}

std::vector<uni::net::EndpointId> const& SlaveConfigManager::config_endpoints() const {
  return _config_endpoints;
}

} // namespace slave
} // namespace uni
