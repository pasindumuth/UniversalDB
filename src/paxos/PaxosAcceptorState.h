#ifndef UNI_PAXOS_PAXOSACCEPTORSTATE_H
#define UNI_PAXOS_PAXOSACCEPTORSTATE_H

#include <unordered_map>
#include <tuple>

#include <paxos/PaxosTypes.h>

namespace uni {
namespace paxos {

// Stores the Acceptor state of a Paxos Instance.
struct PaxosAcceptorState {
  PaxosAcceptorState();

  // The first element is the latest Proposal Number that was sent in a Prepare message, the second
  // element is the Proposal Number of the last accepted value, and the third element is the value
  // of the last accepted value.
  std::tuple<rnd_t, vrnd_t, vval_t> accepted_state;
};

} // namespace paxos
} // namespace uni


#endif // UNI_PAXOS_PAXOSACCEPTORSTATE_H
