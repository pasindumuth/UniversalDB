#include "SlaveKeySpaceManager.h"

namespace uni {
namespace slave {

SlaveKeySpaceManager::SlaveKeySpaceManager(
  uni::async::AsyncQueue& async_queue,
  uni::net::Connections& master_connections,
  uni::paxos::MultiPaxosHandler& multipaxos_handler,
  uni::paxos::PaxosLog& paxos_log):
    _async_queue(async_queue),
    _master_connections(master_connections),
    _multipaxos_handler(multipaxos_handler),
    _paxos_log(paxos_log) {}

} // namespace slave
} // namespace uni
