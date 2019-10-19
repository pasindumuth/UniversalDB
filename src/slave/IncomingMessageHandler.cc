#include "IncomingMessageHandler.h"

#include <logging/log.h>
#include <proto/message.pb.h>

namespace uni {
namespace slave {

using proto::message::MessageWrapper;
using uni::net::IncomingMessage;
using uni::paxos::MultiPaxosHandler;
using uni::slave::ClientRequestHandler;
using uni::slave::FailureDetector;
using uni::slave::LogSyncer;

IncomingMessageHandler::IncomingMessageHandler(
    ClientRequestHandler& request_handler,
    FailureDetector& failure_detector,
    LogSyncer& log_syncer,
    MultiPaxosHandler& multi_paxos_handler)
      : _client_request_handler(request_handler),
        _failure_detector(failure_detector),
        _log_syncer(log_syncer),
        _multi_paxos_handler(multi_paxos_handler) {}

void IncomingMessageHandler::handle(IncomingMessage incoming_message) {
  auto endpoint_id = incoming_message.endpoint_id;
  auto message_wrapper = MessageWrapper();
  message_wrapper.ParseFromString(incoming_message.message);
  if (message_wrapper.has_client_message()) {
    auto const& client_message = message_wrapper.client_message();
    if (client_message.has_request()) {
      LOG(uni::logging::Level::TRACE, "Client Request gotten.")
      _client_request_handler.handle_request(endpoint_id, client_message.request());
    }
  } else if (message_wrapper.has_paxos_message()) {
    LOG(uni::logging::Level::TRACE, "Paxos Message gotten.")
    _multi_paxos_handler.handle_incoming_message(endpoint_id, message_wrapper.paxos_message());
  } else if (message_wrapper.has_slave_message()) {
    auto const& slave_message = message_wrapper.slave_message();
    if (slave_message.has_heartbeat()) {
      LOG(uni::logging::Level::TRACE, "Heartbeat gotten.")
      _failure_detector.handle_heartbeat(endpoint_id);
    } else if (slave_message.has_sync_request()) {
      LOG(uni::logging::Level::TRACE, "Sync Request gotten.")
      _log_syncer.handle_sync_request(endpoint_id, slave_message.sync_request());
    } else if (slave_message.has_sync_response()) {
      LOG(uni::logging::Level::TRACE, "Sync Response gotten.")
      _log_syncer.handle_sync_response(slave_message.sync_response());
    }
  }
}

} // namespace slave
} // namespace uni
