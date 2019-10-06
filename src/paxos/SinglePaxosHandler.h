#ifndef UNI_PAXOS_SINGLEPAXOSHANDLER_H
#define UNI_PAXOS_SINGLEPAXOSHANDLER_H

#include <constants/constants.h>
#include <net/ConnectionsOut.h>
#include <paxos/PaxosAcceptorState.h>
#include <paxos/PaxosLearnerState.h>
#include <paxos/PaxosLog.h>
#include <paxos/PaxosProposerState.h>
#include <paxos/PaxosTypes.h>
#include <proto/message.pb.h>
#include <proto/paxos.pb.h>

namespace uni {
namespace paxos {

// Implements the Single Paxos algorithm. This is primary used in the MultiPaxosHandler
// to reach consensus about what the next PaxosLog entry should be.
// See: http://localhost:3000/projects/universaldb/consensus for more details about
// the Single Paxos Algorithm.
class SinglePaxosHandler {
 public:
  SinglePaxosHandler(
      uni::constants::Constants const& constants,
      uni::net::ConnectionsOut& connections_out,
      uni::paxos::PaxosLog& paxos_log,
      index_t paxos_log_index);

  crnd_t next_proposal_number();

  // For the number of slave servers defined by uni::constants::num_slave_serves, this
  // function computes the smallest number that is a majority. This is simply
  // floor(uni::constants::num_slave_serves) + 1.
  unsigned majority_threshold();

  // We pass in a PaxosMessage from the outside, populated with extra metadata that this
  // class shouldn't be concerned with.
  void propose(
      proto::message::MessageWrapper message_wrapper,
      proto::paxos::PaxosLogEntry const& entry);

  // We pass in a PaxosMessage from the outside, populated with extra metadata that this
  // class shouldn't be concerned with. We also pass in the endpoint_id of the sender
  // of the PREPARE message.
  void prepare(
      proto::message::MessageWrapper message_wrapper,
      uni::net::endpoint_id const& endpoint_id,
      proto::paxos::Prepare const& prepare_message);

  void promise(
      proto::message::MessageWrapper message_wrapper,
      proto::paxos::Promise const& promise_message);

  void accept(
      proto::message::MessageWrapper message_wrapper,
      proto::paxos::Accept const& accept_messge);

  void learn(proto::paxos::Learn const& accept_messge);

 private:
  uni::constants::Constants const& _constants;
  uni::net::ConnectionsOut& _connections_out;
  uni::paxos::PaxosLog& _paxos_log;
  index_t const _paxos_log_index; // The index of the Paxos Log that this Paxos Instance represents.

  PaxosProposerState _proposer_state;
  PaxosAcceptorState _acceptor_state;
  PaxosLearnerState _learner_state;
};

} // paxos
} // uni

#endif //UNI_PAXOS_SINGLEPAXOSHANDLER_H
