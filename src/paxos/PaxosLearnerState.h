#ifndef UNI_PAXOS_PAXOSLEARNERSTATE_H
#define UNI_PAXOS_PAXOSLEARNERSTATE_H

#include <unordered_map>
#include <tuple>
#include <vector>

#include <net/endpoint_id.h>
#include <paxos/PaxosTypes.h>

namespace uni {
namespace paxos {

// Stores the Learner state of a Paxos Instance. This includes all values that were
// received in learn messages, and the number of acceptors to have sent that learn
// value.
struct PaxosLearnerState {
  PaxosLearnerState();

  bool learned;
  // Maps all proposal number that were received in Learn messages to the value associated with that
  // learn message, as well as the count of the number of learn messages that were received with that
  // Proposal Number.
  std::unordered_map<lrnd_t, std::tuple<lval_t, unsigned>> learned_value;
};

} // namespace paxos
} // namespace uni


#endif // UNI_PAXOS_PAXOSLEARNERSTATE_H
