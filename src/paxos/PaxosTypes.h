#ifndef UNI_PAXOS_PAXOSTYPES_H
#define UNI_PAXOS_PAXOSTYPES_H

#include <proto/paxos.pb.h>

namespace uni {
namespace paxos {

// Multipaxos
using index_t = unsigned long; // Paxos instance index

// Proposer
using crnd_t = unsigned; // Proposal number proposed
using cval_t = proto::paxos::PaxosLogEntry; // Value sent for acceptance
using ccnt_t = unsigned; // Count of promises for proposal number

// Acceptor
using rnd_t = unsigned; // Last promised proposal number
using vrnd_t = unsigned; // Last accepted proposal number
using vval_t = proto::paxos::PaxosLogEntry; // Last accepted value

// Learner
using lrnd_t = unsigned; // Proposal number learned
using lval_t = proto::paxos::PaxosLogEntry; // Value learned
using lcnt_t = unsigned; // Count of learned messages for proposal number

} // namespace paxos
} // namespace uni


#endif // UNI_PAXOS_PAXOSTYPES_H
