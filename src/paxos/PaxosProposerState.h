#ifndef UNI_PAXOS_PAXOSPROPOSERSTATE_H
#define UNI_PAXOS_PAXOSPROPOSERSTATE_H

#include <unordered_map>
#include <map>
#include <tuple>
#include <vector>

#include <paxos/PaxosTypes.h>

namespace uni {
namespace paxos {

struct PaxosProposerState {
  PaxosProposerState();

  crnd_t latest;
  // Maps the paxos instance to all the proposals this node has made in that paxos instance.
  // Any new proposals must use a higher proposal number than the latest prosal in the paxos instance.
  // The history of proposals in a paxos instance are in ascending order, where the crnd_t is ascending.
  std::unordered_map<crnd_t, cval_t> proposal;
  // Maps the paxos instance to the the prepare state. The prepare state maps the proposal
  // number to the vrnd_t and vval_t of the acceptors that send a promise message to this node.
  // The vrnd_t and vval_t are the last accepted values of the acceptor, where if the acceptor
  // has never accepted a value, vrnd_t is -1, and vval_t is the empty"".
  std::unordered_map<crnd_t, std::vector<std::tuple<vrnd_t, vval_t>>> prepare_state;
};

} // namespace paxos
} // namespace uni


#endif // UNI_PAXOS_PAXOSPROPOSERSTATE_H
