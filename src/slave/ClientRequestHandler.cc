#include "ClientRequestHandler.h"

#include <iostream>

#include <assert/assert.h>
#include <logging/log.h>
#include <proto/client.pb.h>
#include <proto/message.pb.h>
#include <proto/paxos.pb.h>

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
    // TOOD: fix the logic here
    auto log_entry = PaxosLogEntry();
    log_entry.set_request_id(message.request_id());
    switch (message.request_type()) {
      case ClientRequest::WRITE:
        log_entry.set_type(PaxosLogEntry::WRITE);
        break;
      case ClientRequest::READ:
        log_entry.set_type(PaxosLogEntry::READ);
        break;
      default:
        UNIVERSAL_ASSERT_MESSAGE(false, "Client request type should be READ or WRITE")
    }
    log_entry.set_key(message.key());
    log_entry.set_value(message.value());
    log_entry.set_timestamp(message.timestamp());
    _multi_paxos_handler.propose(log_entry);
    return -1;
  });
}

} // namespace slave
} // namespace uni
