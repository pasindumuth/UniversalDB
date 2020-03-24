#include "MasterIncomingMessageHandler.h"

#include <proto/message.pb.h>

namespace uni {
namespace master {

MasterIncomingMessageHandler::MasterIncomingMessageHandler(
    uni::server::LogSyncer& log_syncer,
    uni::paxos::MultiPaxosHandler& multi_paxos_handler,
    uni::master::GroupConfigManager& group_config_manager,
    uni::master::KeySpaceManager& key_space_manager)
      : _log_syncer(log_syncer),
        _multi_paxos_handler(multi_paxos_handler),
        _group_config_manager(group_config_manager),
        _key_space_manager(key_space_manager) {}

void MasterIncomingMessageHandler::handle(uni::net::IncomingMessage incoming_message) {
  auto endpoint_id = incoming_message.endpoint_id;
  auto message_wrapper = proto::message::MessageWrapper();
  message_wrapper.ParseFromString(incoming_message.message);
  if (message_wrapper.has_client_message()) {
    auto const& client_message = message_wrapper.client_message();
    if (client_message.has_find_key_range_request()) {
      LOG(uni::logging::Level::TRACE2, "Client FindKeyRange message gotten.")
      _key_space_manager.handle_find_key(endpoint_id, client_message.find_key_range_request());
    } else {
      LOG(uni::logging::Level::WARN, "Unkown client message type.")
    }
  } else if (message_wrapper.has_slave_message()) {
    auto const& slave_message = message_wrapper.slave_message();
    if (slave_message.has_key_space_changed()) {
      LOG(uni::logging::Level::TRACE2, "Key Space Changed Message gotten.")
      _key_space_manager.handle_key_space_changed(endpoint_id, slave_message.key_space_changed());
    } else {
      LOG(uni::logging::Level::WARN, "Unkown client message type.")
    }
  } else if (message_wrapper.has_master_message()) {
    auto const& master_message = message_wrapper.master_message();
    if (master_message.has_paxos_message()) {
      LOG(uni::logging::Level::TRACE2, "Paxos Message gotten.")
      _multi_paxos_handler.handle_incoming_message(endpoint_id, master_message.paxos_message());
    } else if (master_message.has_sync_message()) {
      auto sync_message = master_message.sync_message();
      if (sync_message.has_sync_request()) {
        LOG(uni::logging::Level::TRACE2, "Sync Request at gotten.")
        _log_syncer.handle_sync_request(endpoint_id, sync_message.sync_request());
      } else if (sync_message.has_sync_response()) {
        LOG(uni::logging::Level::TRACE2, "Sync Response at gotten.")
        _log_syncer.handle_sync_response(sync_message.sync_response());
      } else {
        LOG(uni::logging::Level::WARN, "Unkown sync message type.")
      }
    } else {
      LOG(uni::logging::Level::WARN, "Unkown master message type.")
    }
  } else {
    LOG(uni::logging::Level::WARN, "Unkown message type in MasterIncomingMessageHandler.")
  }
}

} // namespace master
} // namespace uni
