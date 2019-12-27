#ifndef UNI_PAXOS_PAXOSTYPES_H
#define UNI_PAXOS_PAXOSTYPES_H

#include <common/common.h>
#include <proto/paxos.pb.h>

namespace uni {
namespace paxos {

// Multipaxos
using index_t = uint64_t; // Paxos instance index

// Proposer
using crnd_t = uint32_t; // Proposal number proposed
using cval_t = proto::paxos::PaxosLogEntry; // Value sent for acceptance
using ccnt_t = uint32_t; // Count of promises for proposal number

// Acceptor
using rnd_t = uint32_t; // Last promised proposal number
using vrnd_t = uint32_t; // Last accepted proposal number
using vval_t = proto::paxos::PaxosLogEntry; // Last accepted value

// Learner
using lrnd_t = uint32_t; // Proposal number learned
using lval_t = proto::paxos::PaxosLogEntry; // Value learned
using lcnt_t = uint32_t; // Count of learned messages for proposal number

} // namespace paxos
} // namespace uni


#endif // UNI_PAXOS_PAXOSTYPES_H
