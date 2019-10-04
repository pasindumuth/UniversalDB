#include "PaxosAcceptorState.h"

#include <proto/paxos.pb.h>

namespace uni {
namespace paxos {

using proto::paxos::PaxosLogEntry;

PaxosAcceptorState::PaxosAcceptorState()
  : accepted_state({0, 0, PaxosLogEntry::default_instance()}) {}

} // namespace paxos
} // namespace uni
