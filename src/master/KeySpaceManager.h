#ifndef UNI_MASTER_KEYSPACEMANAGER_H
#define UNI_MASTER_KEYSPACEMANAGER_H

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
namespace master {

class KeySpaceManager {
 public:
  KeySpaceManager(
    uni::async::AsyncQueue& async_queue,
    uni::net::Connections& connections,
    uni::paxos::MultiPaxosHandler& multipaxos_handler,
    uni::paxos::PaxosLog& paxos_log);

  void set_first_config(uni::server::SlaveGroupId group_id);

 private:
   uni::async::AsyncQueue& _async_queue;
   uni::net::Connections& _connections;
   uni::paxos::MultiPaxosHandler& _multipaxos_handler;
   uni::paxos::PaxosLog& _paxos_log;

   struct KeySpace {
      std::vector<uni::server::KeySpaceRange> ranges;
      uint32_t generation;
   };

   struct NewKeySpace {
      std::vector<uni::server::KeySpaceRange> ranges;
      std::vector<uni::server::KeySpaceRange> new_ranges;
      uint32_t generation;
   };

   std::unordered_map<uni::server::SlaveGroupId, std::variant<KeySpace, NewKeySpace>> _slave_group_ranges;
};

} // namespace master
} // namespace uni


#endif // UNI_MASTER_KEYSPACEMANAGER_H
