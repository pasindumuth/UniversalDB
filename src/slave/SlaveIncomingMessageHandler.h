#ifndef UNI_SLAVE_SLAVEINCOMINGMESSAGEHANDLER_H
#define UNI_SLAVE_SLAVEINCOMINGMESSAGEHANDLER_H

#include <memory>
#include <thread>
#include <unordered_map>

#include <boost/asio.hpp>
#include <boost/functional/hash.hpp>

#include <net/ConnectionsIn.h>
#include <net/ConnectionsOut.h>

#include <async/AsyncScheduler.h>
#include <async/TimerAsyncScheduler.h>
#include <common/common.h>
#include <slave/FailureDetector.h>
#include <slave/LogSyncer.h>
#include <slave/HeartbeatTracker.h>
#include <slave/TabletId.h>
#include <slave/TabletParticipant.h>

namespace uni {
namespace slave {

class SlaveIncomingMessageHandler {
 public:
  SlaveIncomingMessageHandler(
    uni::net::ConnectionsIn& client_connections_in,
    uni::net::ConnectionsOut& connections_out,
    uni::slave::FailureDetector& failure_detector,
    uni::slave::HeartbeatTracker& heartbeat_tracker,
    uni::slave::LogSyncer& log_syncer,
    uni::constants::Constants const& constants,
    uni::async::TimerAsyncScheduler& timer_scheduler);

  void handle(uni::net::IncomingMessage incoming_message);

  void addTablet(TabletId tablet_id);

 private:
  uni::net::ConnectionsIn& _client_connections_in;
  uni::net::ConnectionsOut& _connections_out;
  uni::slave::FailureDetector& _failure_detector;
  uni::slave::HeartbeatTracker& _heartbeat_tracker;
  uni::slave::LogSyncer& _log_syncer;
  uni::constants::Constants const& _constants;
  uni::async::TimerAsyncScheduler& _timer_scheduler;

  std::unordered_map<TabletId, std::unique_ptr<uni::slave::TabletParticipant>> _tp_map;

  void forward_message(
    std::string database_id,
    std::string table_id,
    uni::net::IncomingMessage incoming_message);
};

} // namespace slave
} // namespace uni


#endif // UNI_SLAVE_SLAVEINCOMINGMESSAGEHANDLER_H
