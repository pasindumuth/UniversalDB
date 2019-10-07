#ifndef UNI_PAXOS_PAXOSPROPOSERSTATE_H
#define UNI_PAXOS_PAXOSPROPOSERSTATE_H

#include <unordered_map>
#include <map>
#include <tuple>
#include <vector>

#include <paxos/PaxosTypes.h>

namespace uni {
namespace paxos {

// Stores the Proposer state of a Paxos Instance. This includes all proposed
// values and corresponding Proposal Numbers, and all promises made for each proposal.
struct PaxosProposerState {
  PaxosProposerState();

  crnd_t latest;
  // Maps all Proposal Numbers to the values of the Proposal.
  std::unordered_map<crnd_t, cval_t> proposal;
  // Maps all Proposal Numbers to all
  std::unordered_map<crnd_t, std::vector<std::tuple<vrnd_t, vval_t>>> prepare_state;
};

} // namespace paxos
} // namespace uni


#endif // UNI_PAXOS_PAXOSPROPOSERSTATE_H
