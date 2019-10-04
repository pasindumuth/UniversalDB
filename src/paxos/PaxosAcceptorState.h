#ifndef UNI_PAXOS_PAXOSACCEPTORSTATE_H
#define UNI_PAXOS_PAXOSACCEPTORSTATE_H

#include <unordered_map>
#include <tuple>

#include <paxos/PaxosTypes.h>

namespace uni {
namespace paxos {

struct PaxosAcceptorState {
  PaxosAcceptorState();

  // Maps the paxos instance to its accepted state. first int is the latest proposal number
  // this node as accepted. The second int is the proposal number of the most recently accepted
  // value. The PaxosLogEntry is most recently accepted value.
  std::tuple<rnd_t, vrnd_t, vval_t> accepted_state;
};

} // namespace paxos
} // namespace uni


#endif // UNI_PAXOS_PAXOSACCEPTORSTATE_H
