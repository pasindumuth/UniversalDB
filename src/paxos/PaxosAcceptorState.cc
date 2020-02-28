#include "PaxosAcceptorState.h"

#include <proto/paxos.pb.h>

namespace uni {
namespace paxos {

PaxosAcceptorState::PaxosAcceptorState()
  : accepted_state({0, 0, proto::paxos::PaxosLogEntry::default_instance()}) {}

} // namespace paxos
} // namespace uni
