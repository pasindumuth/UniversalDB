#include "KeySpaceManager.h"

namespace uni {
namespace master {

KeySpaceManager::KeySpaceManager(
  uni::async::AsyncQueue& async_queue,
  uni::net::Connections& connections,
  uni::paxos::MultiPaxosHandler& multipaxos_handler,
  uni::paxos::PaxosLog& paxos_log):
    _async_queue(async_queue),
    _connections(connections),
    _multipaxos_handler(multipaxos_handler),
    _paxos_log(paxos_log) {}

} // namespace master
} // namespace uni
