#include "SlaveConfigManager.h"

namespace uni {
namespace slave {

SlaveConfigManager::SlaveConfigManager(
  uni::async::AsyncQueue& async_queue,
  uni::paxos::MultiPaxosHandler& multipaxos_handler,
  uni::paxos::PaxosLog& paxos_log):
    _async_queue(async_queue),
    _multipaxos_handler(multipaxos_handler),
    _paxos_log(paxos_log),
    _config_endpoints{
      {"universal1", 0},
      {"universal2", 0},
      {"universal3", 0},
      {"universal4", 0},
      {"universal5", 0}
    } {}

std::vector<uni::net::EndpointId> SlaveConfigManager::config_endpoints() {
  return _config_endpoints;
}

} // namespace slave
} // namespace uni
