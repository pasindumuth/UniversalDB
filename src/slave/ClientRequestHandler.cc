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
using uni::paxos::MultiPaxosHandler;
using uni::slave::ProposerQueue;

ClientRequestHandler::ClientRequestHandler(
    MultiPaxosHandler& multi_paxos_handler,
    ProposerQueue& proposer_queue)
      : _multi_paxos_handler(multi_paxos_handler),
        _proposer_queue(proposer_queue) {}

void ClientRequestHandler::handle_request(
    uni::net::endpoint_id const& endpoint_id, ClientRequest const& message) {
  _proposer_queue.add_task([this, message](){
    auto log_entry = PaxosLogEntry();
    log_entry.set_value(message.SerializeAsString());
    _multi_paxos_handler.propose(log_entry);
    return -1;
  });
}

} // namespace slave
} // namespace uni
