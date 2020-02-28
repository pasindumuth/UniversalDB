#ifndef UNI_SLAVE_SLAVEINCOMINGMESSAGEHANDLER_H
#define UNI_SLAVE_SLAVEINCOMINGMESSAGEHANDLER_H

#include <memory>
#include <thread>
#include <unordered_map>

#include <boost/asio.hpp>

#include <common/common.h>
#include <slave/LogSyncer.h>
#include <slave/HeartbeatTracker.h>
#include <slave/TabletParticipantManager.h>

namespace uni {
namespace slave {

class SlaveIncomingMessageHandler {
 public:
  SlaveIncomingMessageHandler(
    uni::slave::TabletParticipantManager& participant_manager,
    uni::slave::HeartbeatTracker& heartbeat_tracker,
    uni::slave::LogSyncer& log_syncer);

  void handle(uni::net::IncomingMessage incoming_message);

 private:
  uni::slave::TabletParticipantManager& _participant_manager;
  uni::slave::HeartbeatTracker& _heartbeat_tracker;
  uni::slave::LogSyncer& _log_syncer;

  void forward_message(
    std::string database_id,
    std::string table_id,
    uni::net::IncomingMessage incoming_message);
};

} // namespace slave
} // namespace uni


#endif // UNI_SLAVE_SLAVEINCOMINGMESSAGEHANDLER_H
