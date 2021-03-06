#ifndef UNI_PAXOS_MULTIPAXOSHANDLER_H
#define UNI_PAXOS_MULTIPAXOSHANDLER_H

#include <unordered_map>

#include <common/common.h>
#include <net/EndpointId.h>
#include <paxos/PaxosLog.h>
#include <paxos/PaxosTypes.h>
#include <paxos/SinglePaxosHandler.h>
#include <proto/paxos.pb.h>

namespace uni {
namespace paxos {

/**
 * Implements the MultiPaxos algorithm
 * http://localhost:3000/projects/universaldb/multipaxos. Recall that
 * MultiPaxos is used to add new PaxosLogEntries to the PaxosLog
 * of the MultiPaxos instance.
 */
class MultiPaxosHandler {
 public:
  MultiPaxosHandler(
    uni::paxos::PaxosLog& paxos_log,
    std::function<uni::paxos::SinglePaxosHandler(index_t)> instance_provider);

  // Attempts to add the provided entry into the PaxosLog.
  void propose(proto::paxos::PaxosLogEntry const& entry);

  // Handles all Paxos messages related to this MultiPaxos instance.
  void handle_incoming_message(
      uni::net::EndpointId const& endpoint_id,
      proto::paxos::PaxosMessage const& paxos_messsage);

 private:
  uni::paxos::PaxosLog& _paxos_log;
  std::function<uni::paxos::SinglePaxosHandler(index_t)> _instance_provider;
  std::unordered_map<index_t, uni::paxos::SinglePaxosHandler> _paxos_instances;

  uni::paxos::SinglePaxosHandler& get_instance(index_t index);
};

} // namespace paxos
} // namespace uni


#endif // UNI_PAXOS_MULTIPAXOSHANDLER_H
