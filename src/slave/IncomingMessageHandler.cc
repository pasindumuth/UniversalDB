#include "IncomingMessageHandler.h"

#include <proto/client.pb.h>
#include <proto/message.pb.h>

namespace uni {
namespace slave {

using proto::client::ClientMessage;
using proto::client::ClientResponse;
using proto::message::MessageWrapper;
using uni::net::IncomingMessage;
using uni::paxos::MultiPaxosHandler;
using uni::slave::ClientRequestHandler;
using uni::slave::HeartbeatTracker;
using uni::slave::LogSyncer;

IncomingMessageHandler::IncomingMessageHandler(
    ClientRequestHandler& request_handler,
    HeartbeatTracker& heartbeat_tracker,
    LogSyncer& log_syncer,
    MultiPaxosHandler& multi_paxos_handler)
      : _client_request_handler(request_handler),
        _heartbeat_tracker(heartbeat_tracker),
        _log_syncer(log_syncer),
        _multi_paxos_handler(multi_paxos_handler) {}

void IncomingMessageHandler::handle(IncomingMessage incoming_message) {
  auto endpoint_id = incoming_message.endpoint_id;
  auto message_wrapper = MessageWrapper();
  message_wrapper.ParseFromString(incoming_message.message);
  if (message_wrapper.has_client_message()) {
    auto const& client_message = message_wrapper.client_message();
    if (client_message.has_request()) {
      LOG(uni::logging::Level::TRACE2, "Client Request gotten.")
      _client_request_handler.handle_request(endpoint_id, client_message.request());
    }
  } else if (message_wrapper.has_tablet_message()) {
    auto tablet_message = message_wrapper.tablet_message();
    if (tablet_message.has_paxos_message()) {
      LOG(uni::logging::Level::TRACE2, "Paxos Message gotten.")
      _multi_paxos_handler.handle_incoming_message(endpoint_id, tablet_message.paxos_message());
    } else if (tablet_message.has_sync_message()) {
      auto sync_message = tablet_message.sync_message();
      if (sync_message.has_sync_request()) {
        LOG(uni::logging::Level::TRACE2, "Sync Request gotten.")
        _log_syncer.handle_sync_request(endpoint_id, sync_message.sync_request());
      } else if (sync_message.has_sync_response()) {
        LOG(uni::logging::Level::TRACE2, "Sync Response gotten.")
        _log_syncer.handle_sync_response(sync_message.sync_response());
      }
    }
  } else {
    LOG(uni::logging::Level::WARN, "Unkown message type in IncomingMessageHandler.")
  }
}

} // namespace slave
} // namespace uni
