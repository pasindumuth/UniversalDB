#ifndef UNI_PAXOS_MULTIPAXOSHANDLER_H
#define UNI_PAXOS_MULTIPAXOSHANDLER_H

#include <unordered_map>

#include <net/endpoint_id.h>
#include <paxos/PaxosLog.h>
#include <paxos/PaxosTypes.h>
#include <paxos/SinglePaxosHandler.h>
#include <proto/paxos.pb.h>

namespace uni {
namespace paxos {

class MultiPaxosHandler {
 public:
  MultiPaxosHandler(
      uni::paxos::PaxosLog& paxos_log,
      std::function<uni::paxos::SinglePaxosHandler(index_t)> instance_provider);

  void propose(proto::paxos::PaxosLogEntry const& entry);

  void handle_incoming_message(
      uni::net::endpoint_id const& endpoint_id,
      proto::paxos::PaxosMessage const& paxos_messsage);

 private:
  uni::paxos::PaxosLog& _paxos_log;
  std::function<uni::paxos::SinglePaxosHandler(index_t)> _instance_provider;
  std::unordered_map<index_t, uni::paxos::SinglePaxosHandler> _paxos_instances;

  uni::paxos::SinglePaxosHandler& get_instance(index_t index);
};

} // paxos
} // uni


#endif // UNI_PAXOS_MULTIPAXOSHANDLER_H
