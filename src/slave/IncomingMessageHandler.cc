#include "IncomingMessageHandler.h"

#include <proto/message_client.pb.h>
#include <proto/message.pb.h>

namespace uni {
namespace slave {

IncomingMessageHandler::IncomingMessageHandler(
    uni::slave::ClientRequestHandler& request_handler,
    uni::server::LogSyncer& log_syncer,
    uni::paxos::MultiPaxosHandler& multi_paxos_handler)
      : _client_request_handler(request_handler),
        _log_syncer(log_syncer),
        _multi_paxos_handler(multi_paxos_handler) {}

void IncomingMessageHandler::handle(uni::net::IncomingMessage incoming_message) {
  auto endpoint_id = incoming_message.endpoint_id;
  auto message_wrapper = proto::message::MessageWrapper();
  message_wrapper.ParseFromString(incoming_message.message);
  if (message_wrapper.has_client_message()) {
    auto const& client_message = message_wrapper.client_message();
    if (client_message.has_request()) {
      LOG(uni::logging::Level::TRACE2, "Client Request at IncomingMessageHandler gotten.")
      _client_request_handler.handle_request(endpoint_id, client_message.request());
    } else {
      LOG(uni::logging::Level::WARN, "Unkown client message type.")
    }
  } else if (message_wrapper.has_tablet_message()) {
    auto tablet_message = message_wrapper.tablet_message();
    if (tablet_message.has_paxos_message()) {
      LOG(uni::logging::Level::TRACE2, "Paxos Message at IncomingMessageHandler gotten.")
      _multi_paxos_handler.handle_incoming_message(endpoint_id, tablet_message.paxos_message());
    } else if (tablet_message.has_sync_message()) {
      auto sync_message = tablet_message.sync_message();
      if (sync_message.has_sync_request()) {
        LOG(uni::logging::Level::TRACE2, "Sync Request at IncomingMessageHandler gotten.")
        _log_syncer.handle_sync_request(endpoint_id, sync_message.sync_request());
      } else if (sync_message.has_sync_response()) {
        LOG(uni::logging::Level::TRACE2, "Sync Response at IncomingMessageHandler gotten.")
        _log_syncer.handle_sync_response(sync_message.sync_response());
      } else {
        LOG(uni::logging::Level::WARN, "Unkown sync message type.")
      }
    } else {
      LOG(uni::logging::Level::WARN, "Unkown tablet message type.")
    }
  } else {
    LOG(uni::logging::Level::WARN, "Unkown message type in IncomingMessageHandler.")
  }
}

} // namespace slave
} // namespace uni
