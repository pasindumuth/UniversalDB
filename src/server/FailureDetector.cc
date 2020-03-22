#include "FailureDetector.h"

#include <proto/slave.pb.h>

namespace uni {
namespace server {

FailureDetector::FailureDetector(
    uni::server::HeartbeatTracker& heartbeat_tracker,
    uni::net::Connections& connections,
    uni::async::TimerAsyncScheduler& timer_scheduler,
    std::function<std::vector<uni::net::EndpointId>()> get_endpoints)
      : _heartbeat_tracker(heartbeat_tracker),
        _connections(connections),
        _timer_scheduler(timer_scheduler),
        _get_endpoints(get_endpoints) {
  // Sub-messages are freed when the _message is freed.  
  auto slave_message = new proto::slave::SlaveMessage;
  auto heartbeat_message = new proto::slave::Heartbeat;
  slave_message->set_allocated_heartbeat(heartbeat_message);
  _message.set_allocated_slave_message(slave_message);
  schedule_heartbeat();
}

void FailureDetector::schedule_heartbeat() {
  _timer_scheduler.schedule_repeated([this](){
    _connections.broadcast(_get_endpoints(), _message.SerializeAsString());
    _heartbeat_tracker.increment_counts();
  }, uni::server::HeartbeatTracker::HEARTBEAT_SEND_PERIOD);
}

} // namespace server
} // namespace uni
