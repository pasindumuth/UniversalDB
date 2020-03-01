#ifndef UNI_SLAVE_SLAVEINCOMINGMESSAGEHANDLER_H
#define UNI_SLAVE_SLAVEINCOMINGMESSAGEHANDLER_H

#include <functional>
#include <memory>
#include <thread>
#include <unordered_map>

#include <boost/asio.hpp>

#include <common/common.h>
#include <slave/LogSyncer.h>
#include <slave/HeartbeatTracker.h>
#include <slave/TabletParticipant.h>

namespace uni {
namespace slave {

class SlaveIncomingMessageHandler {
 public:
  SlaveIncomingMessageHandler(
    std::function<uni::custom_unique_ptr<uni::slave::TabletParticipant>(uni::slave::TabletId)> tp_provider,
    uni::slave::HeartbeatTracker& heartbeat_tracker,
    uni::slave::LogSyncer& log_syncer);

  void handle(uni::net::IncomingMessage incoming_message);

  std::unordered_map<TabletId, uni::custom_unique_ptr<uni::slave::TabletParticipant>> const& get_tps() const;

 private:
  std::function<uni::custom_unique_ptr<uni::slave::TabletParticipant>(uni::slave::TabletId)> _tp_provider;
  uni::slave::HeartbeatTracker& _heartbeat_tracker;
  uni::slave::LogSyncer& _log_syncer;

  std::unordered_map<TabletId, uni::custom_unique_ptr<uni::slave::TabletParticipant>> _tp_map;

  void forward_message(
    std::string database_id,
    std::string table_id,
    uni::net::IncomingMessage incoming_message);
};

} // namespace slave
} // namespace uni


#endif // UNI_SLAVE_SLAVEINCOMINGMESSAGEHANDLER_H
