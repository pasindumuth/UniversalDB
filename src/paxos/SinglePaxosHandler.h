#ifndef UNI_PAXOS_SINGLEPAXOSHANDLER_H
#define UNI_PAXOS_SINGLEPAXOSHANDLER_H

#include <functional>
#include <vector>

#include <common/common.h>
#include <constants/constants.h>
#include <net/Connections.h>
#include <net/EndpointId.h>
#include <paxos/PaxosLog.h>
#include <paxos/PaxosTypes.h>
#include <proto/message.pb.h>
#include <proto/paxos.pb.h>
#include <random/Random.h>
#include <slave/TabletId.h>

namespace uni {
namespace paxos {

// Stores the Proposer state of a Paxos Instance. This includes all proposed
// values and corresponding Proposal Numbers, and all promises made for each proposal.
struct PaxosProposerState {
  PaxosProposerState();

  crnd_t latest;
  // Maps all Proposal Numbers to the values of the Proposal.
  std::unordered_map<crnd_t, cval_t> proposal;
  // Maps all Proposal Numbers to all of the Promises made from different Acceptors.
  std::unordered_map<crnd_t, std::vector<std::tuple<vrnd_t, vval_t>>> prepare_state;
};

// Stores the Acceptor state of a Paxos Instance.
struct PaxosAcceptorState {
  PaxosAcceptorState();

  // The first element is the latest Proposal Number that was sent in a Prepare message,
  // the second element is the Proposal Number of the last accepted value, and the third
  // element is the value of the last accepted value.
  std::tuple<rnd_t, vrnd_t, vval_t> accepted_state;
};

// Stores the Learner state of a Paxos Instance. This includes all values that were
// received in Learn messages, and the number of Acceptors to have sent that learned
// value.
struct PaxosLearnerState {
  PaxosLearnerState();

  bool learned;
  // Maps all Proposal Numbers that were received in Learn messages to the value
  // associated with that Learn message, as well as the count of the number of
  // Learn messages that were received with that Proposal Number.
  std::unordered_map<lrnd_t, std::tuple<lval_t, uint32_t>> learned_value;
};

// Implements the Paxos algorithm. This is primarily designed to be used
// as a single instance of the Paxos algorithm in the context of Multipaxos.
// Thus, it keeps track of the paxos_log_index to know where in the PaxosLog
// to insert the learned value when it gets learned.
class SinglePaxosHandler {
 public:
  SinglePaxosHandler(
    uni::constants::Constants const& constants,
    uni::net::Connections& connections,
    uni::paxos::PaxosLog& paxos_log,
    uni::random::Random& random,
    index_t paxos_log_index,
    std::vector<uni::net::EndpointId> config_endpoints,
    std::function<proto::message::MessageWrapper(proto::paxos::PaxosMessage*)> paxos_message_to_wrapper);

  // Returns a value that's strictly greater than the last Proposal Number in
  // _proposer_state. Since that last Proposal Number starts out as 0, this
  // function always returns a value greater than 0.
  crnd_t next_proposal_number();

  // Computes the size of the smallest majority in the Paxos Group. The size of
  // a Paxos Group is uni::constants::num_slave_serves, so the size of the smallest
  // majority is simply floor(uni::constants::num_slave_serves) + 1.
  uint32_t majority_threshold();

  // Initiates a Proposal for this Paxos Instance.
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
  uni::random::Random& _random;
  // The index of the Paxos Log that this Paxos Instance is trying to populate.
  index_t const _paxos_log_index;
  std::vector<uni::net::EndpointId> _config_endpoints;
  std::function<proto::message::MessageWrapper(proto::paxos::PaxosMessage*)> _paxos_message_to_wrapper;

  PaxosProposerState _proposer_state;
  PaxosAcceptorState _acceptor_state;
  PaxosLearnerState _learner_state;
};

} // namespace paxos
} // namespace uni

#endif //UNI_PAXOS_SINGLEPAXOSHANDLER_H
