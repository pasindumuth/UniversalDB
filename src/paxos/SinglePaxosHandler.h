#ifndef UNI_PAXOS_SINGLEPAXOSHANDLER_H
#define UNI_PAXOS_SINGLEPAXOSHANDLER_H

#include <functional>
#include <vector>

#include <common/common.h>
#include <constants/constants.h>
#include <net/Connections.h>
#include <net/EndpointId.h>
#include <paxos/PaxosAcceptorState.h>
#include <paxos/PaxosLearnerState.h>
#include <paxos/PaxosLog.h>
#include <paxos/PaxosProposerState.h>
#include <paxos/PaxosTypes.h>
#include <proto/message.pb.h>
#include <proto/paxos.pb.h>
#include <slave/TabletId.h>

namespace uni {
namespace paxos {

/**
 * Implements the Single Paxos algorithm
 * http://localhost:3000/projects/universaldb/consensus. This is primarily
 * designed to be used for a Single Paxos Instance in the MultiPaxos algorithm
 * http://localhost:3000/projects/universaldb/multipaxos. This is why it keeps
 * track of the paxos_log_index, to know where in the PaxosLog to insert
 * the learned value when it gets learned.
 */
class SinglePaxosHandler {
 public:
  SinglePaxosHandler(
      uni::constants::Constants const& constants,
      uni::net::Connections& connections,
      uni::paxos::PaxosLog& paxos_log,
      index_t paxos_log_index,
      std::function<std::vector<uni::net::EndpointId>()> get_endpoints,
      std::function<proto::message::MessageWrapper(proto::paxos::PaxosMessage*)> paxos_message_to_wrapper);

  // Returns a value that's strictly greater than the last proposal number in
  // _proposer_state. Since that last proposal number starts out as 0, this
  // function always returns a value greater than 0.
  crnd_t next_proposal_number();

  // Computes the size of the smallest majority in the Paxos Group. The size of
  // a Paxos Group is uni::constants::num_slave_serves, so the size of the smallest
  // majority is simply floor(uni::constants::num_slave_serves) + 1.
  uint32_t majority_threshold();

  // Initiates a proposal for this Paxos Instance.
  void propose(proto::paxos::PaxosLogEntry const& entry);

  // Handles a Prepare message for this Paxos Instance.
  void prepare(uni::net::EndpointId const& endpoint_id, proto::paxos::Prepare const& prepare_message);

  // Handles a Promise message for this Paxos Instance.
  void promise(proto::paxos::Promise const& promise_message);

  // Handles an Accept message for this Paxos Instance.
  void accept(proto::paxos::Accept const& accept_messge);

  // Handles a Learn message for this Paxos Instance.
  void learn(proto::paxos::Learn const& accept_messge);

 private:
  uni::constants::Constants const& _constants;
  uni::net::Connections& _connections;
  uni::paxos::PaxosLog& _paxos_log;
  // The index of the Paxos Log that this Paxos Instance is trying to populate.
  index_t const _paxos_log_index;
  std::function<std::vector<uni::net::EndpointId>()> _get_endpoints;
  std::function<proto::message::MessageWrapper(proto::paxos::PaxosMessage*)> _paxos_message_to_wrapper;

  PaxosProposerState _proposer_state;
  PaxosAcceptorState _acceptor_state;
  PaxosLearnerState _learner_state;
};

} // paxos
} // uni

#endif //UNI_PAXOS_SINGLEPAXOSHANDLER_H
