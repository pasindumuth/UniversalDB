#include "FailureDetector.h"

#include <proto/slave.pb.h>

namespace uni {
namespace slave {

FailureDetector::FailureDetector(
    uni::slave::HeartbeatTracker& heartbeat_tracker,
    uni::net::ConnectionsOut& connections_out,
    uni::async::TimerAsyncScheduler& timer_scheduler)
      : _heartbeat_tracker(heartbeat_tracker),
        _connections_out(connections_out),
        _timer_scheduler(timer_scheduler) {
  // Sub-messages are freed when the _message is freed.  
  auto slave_message = new proto::slave::SlaveMessage;
  auto heartbeat_message = new proto::slave::Heartbeat;
  slave_message->set_allocated_heartbeat(heartbeat_message);
  _message.set_allocated_slave_message(slave_message);
  schedule_heartbeat();
}

void FailureDetector::schedule_heartbeat() {
  _timer_scheduler.schedule_repeated([this](){
    _connections_out.broadcast(_message.SerializeAsString());
    _heartbeat_tracker.increment_counts();
  }, uni::slave::HeartbeatTracker::HEARTBEAT_SEND_PERIOD);
}

} // namespace slave
} // namespace uni
