#include "ClientRequestHandler.h"

#include <iostream>

#include <assert/assert.h>
#include <logging/log.h>
#include <proto/client.pb.h>
#include <proto/message.pb.h>
#include <proto/paxos.pb.h>
#include <utils/pbutil.h>

namespace uni {
namespace slave {

using proto::client::ClientMessage;
using proto::client::ClientRequest;
using proto::client::ClientResponse;
using proto::message::MessageWrapper;
using proto::paxos::PaxosLogEntry;
using uni::paxos::MultiPaxosHandler;
using uni::paxos::PaxosLog;
using uni::slave::ProposerQueue;

ClientRequestHandler::ClientRequestHandler(
    MultiPaxosHandler& multi_paxos_handler,
    PaxosLog& paxos_log,
    ProposerQueue& proposer_queue)
      : _multi_paxos_handler(multi_paxos_handler),
        _paxos_log(paxos_log),
        _proposer_queue(proposer_queue) {
  _paxos_log.add_callback([this](PaxosLogEntry entry){
    if (entry.request_id().value().length() > 0) {
      _request_id_set.insert(entry.request_id().value());
    }
  });
}

// TODO finish this
void ClientRequestHandler::handle_request(
    std::function<void(ClientResponse*)> response_callback,
    ClientRequest const& message) {
  auto retry_count = std::make_shared<int>(0);
  _proposer_queue.add_task([this, retry_count, response_callback, message](){
    if (_request_id_set.find(message.request_id()) != _request_id_set.end()) {
      // The request was fullfilled in the last retry
      auto client_response = new ClientResponse();
      client_response->set_error_code(ClientResponse::SUCCESS);
      response_callback(client_response);
    } else if (*retry_count == 3) { // TODO make this into a constant
      // Maximum number of retries have been reached
      auto client_response = new ClientResponse();
      client_response->set_error_code(ClientResponse::ERROR);
      response_callback(client_response);
    }
    
    auto log_entry = PaxosLogEntry();
    log_entry.set_allocated_request_id(uni::utils::pb::string(message.request_id()));
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
    log_entry.set_allocated_key(uni::utils::pb::string(message.key()));
    log_entry.set_allocated_value(uni::utils::pb::string(message.value()));
    log_entry.set_allocated_timestamp(uni::utils::pb::uint64(message.timestamp()));
    _multi_paxos_handler.propose(log_entry);
    return -1;
  });
}

} // namespace slave
} // namespace uni
