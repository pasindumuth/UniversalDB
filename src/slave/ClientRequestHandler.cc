#include "ClientRequestHandler.h"

#include <iostream>

#include <assert/assert.h>
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
using uni::paxos::index_t;
using uni::paxos::MultiPaxosHandler;
using uni::paxos::PaxosLog;
using uni::slave::KVStore;
using uni::slave::ProposerQueue;

ClientRequestHandler::ClientRequestHandler(
    MultiPaxosHandler& multi_paxos_handler,
    PaxosLog& paxos_log,
    ProposerQueue& proposer_queue,
    KVStore& kvstore,
    std::function<void(uni::net::endpoint_id, ClientResponse*)> respond)
      : _multi_paxos_handler(multi_paxos_handler),
        _paxos_log(paxos_log),
        _proposer_queue(proposer_queue),
        _kvstore(kvstore),
        _respond(respond) {
  _paxos_log.add_callback([this](index_t index, PaxosLogEntry entry){
    if (entry.request_id().value().length() > 0) {
      _request_id_map.insert({ entry.request_id().value(), index });
    }
  });
}

// TODO finish this. We have to actually update the KVStore. We have to actually
// check if a write is valid.
void ClientRequestHandler::handle_request(
    uni::net::endpoint_id endpoint_id,
    ClientRequest const& message) {
  auto retry_count = std::make_shared<int>(0);
  _proposer_queue.add_task([this, retry_count, message, endpoint_id](){
    auto entry_index = _request_id_map.find(message.request_id());
    if (entry_index != _request_id_map.end()) {
      // The request was fullfilled in the last retry
      auto client_response = new ClientResponse();
      client_response->set_error_code(ClientResponse::SUCCESS);
      if (message.request_type() == ClientRequest::READ) {
        // Populate the value of the read
        auto entry = _paxos_log.get_entry(entry_index->second);
        client_response->set_allocated_value(uni::utils::pb::string(entry.get().value()));
      }
      _respond(endpoint_id, client_response);
      return -1;
    } else if (*retry_count == 3) { // TODO make this into a constant
      // Maximum number of retries have been reached
      auto client_response = new ClientResponse();
      client_response->set_error_code(ClientResponse::ERROR);
      _respond(endpoint_id, client_response);
      return -1;
    }

    if (message.request_type() == ClientRequest::WRITE) {
      // If the write is illegal, then return an error
      auto lat = _kvstore.read_lat(message.key().value());
      if (message.timestamp().value() <= lat) {
        // The timestamp trying to be inserted to is not strictly greater than lat
        auto client_response = new ClientResponse();
        client_response->set_error_code(ClientResponse::ERROR);
        _respond(endpoint_id, client_response);
        return -1;
      }
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
    *retry_count += 1;
    return 100; // TODO make this into a constant
  });
}

} // namespace slave
} // namespace uni
