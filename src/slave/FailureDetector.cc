#include "FailureDetector.h"

#include <memory>

#include <proto/slave.pb.h>

namespace uni {
namespace slave {

using uni::net::ConnectionsOut;
using uni::async::TimerAsyncScheduler;

FailureDetector::FailureDetector(
    uni::constants::Constants const& constants,
    ConnectionsOut& connections_out,
    TimerAsyncScheduler& timer_scheduler)
      : _constants(constants),
        _connections_out(connections_out),
        _timer_scheduler(timer_scheduler) {
  // Sub-messages are freed when the _message is freed.  
  auto slave_message = new proto::slave::SlaveMessage;
  auto heartbeat_message = new proto::slave::Heartbeat;
  slave_message->set_allocated_heartbeat(heartbeat_message);
  _message.set_allocated_slave_message(slave_message);
}

void FailureDetector::schedule_heartbeat() {
  _timer_scheduler.schedule_repeated([this](){
     _connections_out.broadcast(_message.SerializeAsString());
  }, _constants.heartbeat_wait_ms);
}

void FailureDetector::handle_heartbeat(uni::net::endpoint_id endpoint_id) {
  auto it = heartbeat_count.find(endpoint_id);
  if (it == heartbeat_count.end()) {
    heartbeat_count.insert({endpoint_id, 0});
  } else {
    it->second++;
  }
}

} // namespace slave
} // namespace uni
