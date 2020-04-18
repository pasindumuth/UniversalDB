#include "ClientRequestHandler.h"

#include <iostream>

#include <assert/assert.h>
#include <proto/message.pb.h>
#include <proto/paxos.pb.h>
#include <proto/paxos_tablet.pb.h>
#include <utils/pbutil.h>

namespace uni {
namespace slave {

int const ClientRequestHandler::RETRY_LIMIT = 3;
int const ClientRequestHandler::WAIT_FOR_PAXOS = 100;

ClientRequestHandler::ClientRequestHandler(
    uni::paxos::MultiPaxosHandler& multi_paxos_handler,
    uni::paxos::PaxosLog& paxos_log,
    uni::async::AsyncQueue& async_queue,
    uni::slave::KVStore& kvstore,
    std::function<void(uni::net::EndpointId, proto::client::ClientResponse*)> respond)
      : _multi_paxos_handler(multi_paxos_handler),
        _paxos_log(paxos_log),
        _async_queue(async_queue),
        _kvstore(kvstore),
        _respond(respond) {
  _paxos_log.add_callback(
    proto::paxos::PaxosLogEntry::EntryContentCase::kRead,
    [this](uni::paxos::index_t index, proto::paxos::PaxosLogEntry entry) {
      auto read_entry = entry.read();
      _request_id_map.insert({ read_entry.request_id().value(), index });
      _kvstore.read(read_entry.key().value(), read_entry.timestamp().value());
  });

  _paxos_log.add_callback(
    proto::paxos::PaxosLogEntry::EntryContentCase::kWrite,
    [this](uni::paxos::index_t index, proto::paxos::PaxosLogEntry entry) {
      auto write_entry = entry.write();
      _request_id_map.insert({ write_entry.request_id().value(), index });
      _kvstore.write(write_entry.key().value(), write_entry.value().value(), write_entry.timestamp().value());
  });
}

// The way this works is that _request_id_map holds onto the request_ids of all
// requests that have been sent. When this method is run, we first check to see if
// the request has already been satisfied. This is possible if the client had retried,
// perhaps with a different slave. Notice that we do everything asynchronously via
// the propser queue. But as an optimization, before shecheduling anything, we might
// be able to check the _request_id_map to see if the request was already fulfilled,
// and then avoid scheduling it to happen asynchronously (which might take a while
// if there are already blocking jobs in the proposer queue).
void ClientRequestHandler::handle_request(
    uni::net::EndpointId endpoint_id,
    proto::client::ClientRequest const& message) {
  auto retry_count = std::make_shared<int>(0);
  _async_queue.add_task([this, retry_count, message, endpoint_id](){
    auto entry_index = _request_id_map.find(message.request_id());
    if (entry_index != _request_id_map.end()) {
      // The request was fullfilled in the last retry
      auto client_response = new proto::client::ClientResponse();
      if (message.request_type() == proto::client::ClientRequest::READ) {
        // Populate the value of the read
        auto read_entry = _paxos_log.get_entry(entry_index->second).get().read();
        auto value = _kvstore.read(read_entry.key().value(), read_entry.timestamp().value());
        if (value) {
          // The value exists and the read was a success
          client_response->set_error_code(proto::client::Code::SUCCESS);
          client_response->set_allocated_value(uni::utils::pb::string(value.get()));
        } else {
          // The value doesn't exist and the read was a error
          client_response->set_error_code(proto::client::Code::ERROR);
        }
      } else if (message.request_type() == proto::client::ClientRequest::WRITE) {
        client_response->set_error_code(proto::client::Code::SUCCESS);
      } else {
        UNIVERSAL_TERMINATE("Client request type should be READ or WRITE")
      }
      _respond(endpoint_id, client_response);
      return uni::async::AsyncQueue::TERMINATE;
    } else if (*retry_count == RETRY_LIMIT) {
      // Maximum number of retries have been reached
      auto client_response = new proto::client::ClientResponse();
      client_response->set_error_code(proto::client::Code::ERROR);
      _respond(endpoint_id, client_response);
      return uni::async::AsyncQueue::TERMINATE;
    }

    if (message.request_type() == proto::client::ClientRequest::WRITE) {
      // If the write is illegal, then return an error
      auto lat = _kvstore.read_lat(message.key().value());
      if (message.timestamp().value() <= lat) {
        // The timestamp trying to be inserted to is not strictly greater than lat
        auto client_response = new proto::client::ClientResponse();
        client_response->set_error_code(proto::client::Code::ERROR);
        _respond(endpoint_id, client_response);
        return uni::async::AsyncQueue::TERMINATE;
      }
    }

    auto log_entry = proto::paxos::PaxosLogEntry();
    if (message.request_type() == proto::client::ClientRequest::WRITE) {
      auto write = new proto::paxos::tablet::Write();
      write->set_allocated_request_id(uni::utils::pb::string(message.request_id()));
      write->set_allocated_key(uni::utils::pb::string(message.key()));
      write->set_allocated_value(uni::utils::pb::string(message.value()));
      write->set_allocated_timestamp(uni::utils::pb::uint64(message.timestamp()));
      log_entry.set_allocated_write(write);
    } else if (message.request_type() == proto::client::ClientRequest::READ) {
      auto read = new proto::paxos::tablet::Read();
      read->set_allocated_request_id(uni::utils::pb::string(message.request_id()));
      read->set_allocated_key(uni::utils::pb::string(message.key()));
      read->set_allocated_timestamp(uni::utils::pb::uint64(message.timestamp()));
      log_entry.set_allocated_read(read);
    } else {
      UNIVERSAL_TERMINATE("Client request type should be READ or WRITE")
    }
    _multi_paxos_handler.propose(log_entry);
    *retry_count += 1;
    return WAIT_FOR_PAXOS;
  });
}

} // namespace slave
} // namespace uni
