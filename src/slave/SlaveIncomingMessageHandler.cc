#include "SlaveIncomingMessageHandler.h"

#include <common/common.h>
#include <net/EndpointId.h>
#include <net/IncomingMessage.h>
#include <proto/client.pb.h>
#include <proto/master.pb.h>
#include <proto/message.pb.h>
#include <proto/tablet.pb.h>
#include <server/LogSyncer.h>
#include <slave/TabletId.h>

namespace uni {
namespace slave {

SlaveIncomingMessageHandler::SlaveIncomingMessageHandler(
  TabletParticipantManager& participant_manager,
  uni::server::HeartbeatTracker& heartbeat_tracker,
  uni::server::LogSyncer& log_syncer,
  uni::slave::SlaveKeySpaceManager& key_space_manager,
  uni::paxos::MultiPaxosHandler& multipaxos_handler)
  : _participant_manager(participant_manager),
    _heartbeat_tracker(heartbeat_tracker),
    _log_syncer(log_syncer),
    _key_space_manager(key_space_manager),
    _multipaxos_handler(multipaxos_handler) {}

void SlaveIncomingMessageHandler::handle(uni::net::IncomingMessage incoming_message) {
  auto endpoint_id = incoming_message.endpoint_id;
  auto message_wrapper = proto::message::MessageWrapper();
  message_wrapper.ParseFromString(incoming_message.message);
  if (message_wrapper.has_client_message()) {
    auto const& client_message = message_wrapper.client_message();
    if (client_message.has_request()) {
      LOG(uni::logging::Level::TRACE2, "Client Request at SlaveIncomingMessageHandler gotten.")
      _participant_manager.handle(incoming_message);
    } else {
      LOG(uni::logging::Level::WARN, "Unkown client message type.")
    }
  } else if (message_wrapper.has_tablet_message()) {
    LOG(uni::logging::Level::TRACE2, "TabletMessage at SlaveIncomingMessageHandler gotten.")
    auto tablet_message = message_wrapper.tablet_message();
    _participant_manager.handle(incoming_message);
  } else if (message_wrapper.has_slave_message()) {
    auto const& slave_message = message_wrapper.slave_message();
    if (slave_message.has_heartbeat()) {
      LOG(uni::logging::Level::TRACE2, "Heartbeat at SlaveIncomingMessageHandler gotten.")
      _heartbeat_tracker.handle_heartbeat(endpoint_id);
    } else if (slave_message.has_sync_message()) {
      auto sync_message = slave_message.sync_message();
      if (sync_message.has_sync_request()) {
        LOG(uni::logging::Level::TRACE2, "Sync Request at SlaveIncomingMessageHandler gotten.")
        _log_syncer.handle_sync_request(endpoint_id, sync_message.sync_request());
      } else if (sync_message.has_sync_response()) {
        LOG(uni::logging::Level::TRACE2, "Sync Response at SlaveIncomingMessageHandler gotten.")
        _log_syncer.handle_sync_response(sync_message.sync_response());
      } else {
        LOG(uni::logging::Level::WARN, "Unkown sync message type.")
      }
    } else if (slave_message.has_paxos_message()) {
      LOG(uni::logging::Level::TRACE2, "Paxos Message gotten.")
      _multipaxos_handler.handle_incoming_message(endpoint_id, slave_message.paxos_message());
    } else {
      LOG(uni::logging::Level::WARN, "Unkown slave message type.")
    }
  } else if (message_wrapper.has_master_message()) {
    auto const& master_message = message_wrapper.master_message();
    if (master_message.has_key_space_selected()) {
      LOG(uni::logging::Level::TRACE2, "New Key Space Selected gotten.")
      _key_space_manager.handle_key_space_change(endpoint_id, master_message.key_space_selected());
    } else {
      LOG(uni::logging::Level::WARN, "Unkown master message type.")
    }
  } else {
    LOG(uni::logging::Level::WARN, "Unkown message type in SlaveIncomingMessageHandler.")
  }
}

} // namespace slave
} // namespace uni
