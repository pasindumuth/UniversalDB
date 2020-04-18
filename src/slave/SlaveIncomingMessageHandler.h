#ifndef UNI_SLAVE_SLAVEINCOMINGMESSAGEHANDLER_H
#define UNI_SLAVE_SLAVEINCOMINGMESSAGEHANDLER_H

#include <functional>
#include <memory>
#include <thread>
#include <unordered_map>

#include <common/common.h>
#include <paxos/MultiPaxosHandler.h>
#include <server/LogSyncer.h>
#include <server/HeartbeatTracker.h>
#include <slave/SlaveKeySpaceManager.h>
#include <slave/TabletParticipantManager.h>

namespace uni {
namespace slave {

class SlaveIncomingMessageHandler {
 public:
  SlaveIncomingMessageHandler(
    TabletParticipantManager& participant_manager,
    uni::server::HeartbeatTracker& heartbeat_tracker,
    uni::server::LogSyncer& log_syncer,
    uni::slave::SlaveKeySpaceManager& key_space_manager,
    uni::paxos::MultiPaxosHandler& multipaxos_handler);

  void handle(uni::net::IncomingMessage incoming_message);

  std::unordered_map<TabletId, uni::custom_unique_ptr<uni::slave::TabletParticipant>> const& get_tps() const;

 private:
  TabletParticipantManager& _participant_manager;
  uni::server::HeartbeatTracker& _heartbeat_tracker;
  uni::server::LogSyncer& _log_syncer;
  uni::slave::SlaveKeySpaceManager& _key_space_manager;
  uni::paxos::MultiPaxosHandler& _multipaxos_handler;
};

} // namespace slave
} // namespace uni


#endif // UNI_SLAVE_SLAVEINCOMINGMESSAGEHANDLER_H
