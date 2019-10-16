#include "ClientRequestHandler.h"

#include <iostream>

#include <logging/log.h>
#include <proto/paxos.pb.h>
#include <proto/message.pb.h>

namespace uni {
namespace slave {

using proto::client::ClientRequest;
using proto::message::MessageWrapper;
using proto::paxos::PaxosLogEntry;
using uni::async::TimerAsyncScheduler;
using uni::paxos::MultiPaxosHandler;

ClientRequestHandler::ClientRequestHandler(
    MultiPaxosHandler& multi_paxos_handler)
      : _multi_paxos_handler(multi_paxos_handler) {}

void ClientRequestHandler::handle_request(
    uni::net::endpoint_id const& endpoint_id, ClientRequest const& message) {
  LOG(uni::logging::Level::TRACE, "Client request gotten: " + message.DebugString())
  auto log_entry = PaxosLogEntry();
  log_entry.set_value(message.SerializeAsString());
  _multi_paxos_handler.propose(log_entry);
}

} // namespace slave
} // namespace uni
