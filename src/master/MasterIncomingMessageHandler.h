#ifndef UNI_MASTER_MASTERINCOMINGMESSAGEHANDLER_H
#define UNI_MASTER_MASTERINCOMINGMESSAGEHANDLER_H

#include <common/common.h>
#include <master/GroupConfigManager.h>
#include <master/KeySpaceManager.h>
#include <net/IncomingMessage.h>
#include <paxos/MultiPaxosHandler.h>
#include <server/HeartbeatTracker.h>
#include <server/LogSyncer.h>

namespace uni {
namespace master {

class MasterIncomingMessageHandler {
 public:
  MasterIncomingMessageHandler(
      uni::server::LogSyncer& log_syncer,
      uni::paxos::MultiPaxosHandler& multi_paxos_handler,
      uni::master::GroupConfigManager& group_config_manager,
      uni::master::KeySpaceManager& key_space_manager);

  void handle(uni::net::IncomingMessage incoming_message);

 private:
  uni::server::LogSyncer& _log_syncer;
  uni::paxos::MultiPaxosHandler& _multi_paxos_handler;
  uni::master::GroupConfigManager& _group_config_manager;
  uni::master::KeySpaceManager& _key_space_manager;
};

} // namespace master
} // namespace uni


#endif // UNI_MASTER_MASTERINCOMINGMESSAGEHANDLER_H
