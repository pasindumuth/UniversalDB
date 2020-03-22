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
}

} // namespace master
} // namespace uni
