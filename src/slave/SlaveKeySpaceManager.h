#ifndef UNI_SLAVE_SLAVEKEYSPACEMANAGER_H
#define UNI_SLAVE_SLAVEKEYSPACEMANAGER_H

#include <unordered_map>
#include <variant>
#include <vector>

#include <common/common.h>

#include <async/AsyncQueue.h>
#include <net/Connections.h>
#include <paxos/MultiPaxosHandler.h>
#include <proto/master.pb.h>
#include <server/KeySpaceRange.h>
#include <server/SlaveGroupId.h>
#include <slave/TabletParticipantManager.h>

namespace uni {
namespace slave {

class SlaveKeySpaceManager {
 public:
  static int const WAIT_FOR_PAXOS;
  
  SlaveKeySpaceManager(
    uni::async::AsyncQueue& async_queue,
    uni::net::Connections& master_connections,
    uni::paxos::MultiPaxosHandler& multipaxos_handler,
    uni::paxos::PaxosLog& paxos_log,
    uni::slave::TabletParticipantManager& participant_manager);

  void handle_key_space_change(
    uni::net::EndpointId endpoint_id,
    proto::master::NewKeySpaceSelected const& message);

 private:
  uni::async::AsyncQueue& _async_queue;
  uni::net::Connections& _master_connections;
  uni::paxos::MultiPaxosHandler& _multipaxos_handler;
  uni::paxos::PaxosLog& _paxos_log;
  uni::slave::TabletParticipantManager& _participant_manager;

  struct KeySpace {
    std::vector<uni::server::KeySpaceRange> ranges;
    uint32_t generation;
  };

  KeySpace _ranges;
};

} // namespace slave
} // namespace uni


#endif // UNI_SLAVE_SLAVEKEYSPACEMANAGER_H
