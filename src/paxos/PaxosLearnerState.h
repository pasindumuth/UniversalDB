#ifndef UNI_PAXOS_PAXOSLEARNERSTATE_H
#define UNI_PAXOS_PAXOSLEARNERSTATE_H

#include <unordered_map>
#include <tuple>
#include <vector>

#include <net/endpoint_id.h>
#include <paxos/PaxosTypes.h>

namespace uni {
namespace paxos {

struct PaxosLearnerState {
  PaxosLearnerState();

  bool learned;
  // The keys are the proposal numbers that have been recieved by this Slave in a Learn message.
  // Each key is mapped to the value in the learn message, along with a count of all of the Acceptors to
  // have sent a Learn message with this proposal number. Remember that in Paxos, if an Acceptor accepts
  // a <vrnd, vval> pair, then vval is the only value that can appear for vrnd. Hence why the values of the
  // map below consist of only a tuple, and not an array of tuples.
  std::unordered_map<lrnd_t, std::tuple<lval_t, unsigned>> learned_value;
};

} // namespace paxos
} // namespace uni


#endif // UNI_PAXOS_PAXOSLEARNERSTATE_H
