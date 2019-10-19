#include "FailureDetector.h"

#include <sstream>
#include <memory>

#include <logging/log.h>
#include <proto/slave.pb.h>

namespace uni {
namespace slave {

using uni::async::TimerAsyncScheduler;
using uni::constants::Constants;
using uni::net::ConnectionsOut;

FailureDetector::FailureDetector(
    Constants const& constants,
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
    for (auto& [endpoint, count] : _heartbeat_count) {
      count++;
    }
  }, _constants.heartbeat_period);
}

void FailureDetector::handle_heartbeat(uni::net::endpoint_id endpoint_id) {
  auto it = _heartbeat_count.find(endpoint_id);
  if (it == _heartbeat_count.end()) {
    _heartbeat_count.insert({endpoint_id, 0});
  } else {
    it->second = 0;
  }
}

std::vector<uni::net::endpoint_id> FailureDetector::alive_endpoints() {
  auto endpoints = std::vector<uni::net::endpoint_id>();
  auto it = _heartbeat_count.begin();
  // Iterate through all heartbeat counts, removing the endpoints that are
  // dead, and populating the `endpoints` variable with the endpoints
  // that are still alive.
  while (it != _heartbeat_count.end()) {
    if (it->second >= _constants.heartbeat_failure_threshold) {
      it = _heartbeat_count.erase(it);
    } else {
      endpoints.push_back(it->first);
      it = std::next(it);
    }
  }
  return endpoints;
}

boost::optional<uni::net::endpoint_id> FailureDetector::leader_endpoint_id() {
  auto endpoints = alive_endpoints();
  if (endpoints.size() > 0) {
    return alive_endpoints()[0];
  }
  return boost::none;
}

void FailureDetector::debug_print() {
  auto ss = std::stringstream();
  ss << "==============================" << std::endl;
  for (auto const& [endpoint, count] : _heartbeat_count) {
    ss << endpoint.ip_string << ", " << std::to_string(count) << std::endl;
  }
  ss << "==============================" << std::endl;
  LOG(uni::logging::Level::DEBUG, ss.str());
}

} // namespace slave
} // namespace uni
