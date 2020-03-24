#ifndef UNI_SLAVE_SLAVEINCOMINGMESSAGEHANDLER_H
#define UNI_SLAVE_SLAVEINCOMINGMESSAGEHANDLER_H

#include <functional>
#include <memory>
#include <thread>
#include <unordered_map>

#include <boost/asio.hpp>

#include <common/common.h>
#include <paxos/MultiPaxosHandler.h>
#include <server/LogSyncer.h>
#include <server/HeartbeatTracker.h>
#include <slave/SlaveKeySpaceManager.h>
#include <slave/TabletParticipant.h>

namespace uni {
namespace slave {

class SlaveIncomingMessageHandler {
 public:
  SlaveIncomingMessageHandler(
    std::function<uni::custom_unique_ptr<uni::slave::TabletParticipant>(uni::slave::TabletId)> tp_provider,
    uni::server::HeartbeatTracker& heartbeat_tracker,
    uni::server::LogSyncer& log_syncer,
    uni::slave::SlaveKeySpaceManager& key_space_manager,
    uni::paxos::MultiPaxosHandler& multipaxos_handler);

  void handle(uni::net::IncomingMessage incoming_message);

  std::unordered_map<TabletId, uni::custom_unique_ptr<uni::slave::TabletParticipant>> const& get_tps() const;

 private:
  std::function<uni::custom_unique_ptr<uni::slave::TabletParticipant>(uni::slave::TabletId)> _tp_provider;
  uni::server::HeartbeatTracker& _heartbeat_tracker;
  uni::server::LogSyncer& _log_syncer;
  uni::slave::SlaveKeySpaceManager& _key_space_manager;
  uni::paxos::MultiPaxosHandler& _multipaxos_handler;

  std::unordered_map<TabletId, uni::custom_unique_ptr<uni::slave::TabletParticipant>> _tp_map;

  void forward_message(
    std::string database_id,
    std::string table_id,
    uni::net::IncomingMessage incoming_message);
};

} // namespace slave
} // namespace uni


#endif // UNI_SLAVE_SLAVEINCOMINGMESSAGEHANDLER_H
