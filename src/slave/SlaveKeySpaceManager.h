#ifndef UNI_SLAVE_SLAVEKEYSPACEMANAGER_H
#define UNI_SLAVE_SLAVEKEYSPACEMANAGER_H

#include <unordered_map>
#include <variant>
#include <vector>

#include <common/common.h>

#include <async/AsyncQueue.h>
#include <net/Connections.h>
#include <paxos/MultiPaxosHandler.h>
#include <server/KeySpaceRange.h>
#include <server/SlaveGroupId.h>

namespace uni {
namespace slave {

class SlaveKeySpaceManager {
 public:
  SlaveKeySpaceManager(
    uni::async::AsyncQueue& async_queue,
    uni::net::Connections& master_connections,
    uni::paxos::MultiPaxosHandler& multipaxos_handler,
    uni::paxos::PaxosLog& paxos_log);

 private:
   uni::async::AsyncQueue& _async_queue;
   uni::net::Connections& _master_connections;
   uni::paxos::MultiPaxosHandler& _multipaxos_handler;
   uni::paxos::PaxosLog& _paxos_log;

   struct KeySpace {
      std::vector<uni::server::KeySpaceRange> ranges;
      uint32_t generation;
   };

   std::variant<KeySpace> _ranges;
};

} // namespace slave
} // namespace uni


#endif // UNI_SLAVE_SLAVEKEYSPACEMANAGER_H
