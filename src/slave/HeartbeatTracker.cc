#include "HeartbeatTracker.h"

#include <sstream>

namespace uni {
namespace slave {

uint32_t const HeartbeatTracker::HEARTBEAT_FAILURE_COUNT = 4;
uint32_t const HeartbeatTracker::HEARTBEAT_SEND_PERIOD = 1000;

HeartbeatTracker::HeartbeatTracker() {}

void HeartbeatTracker::increment_counts() {
  for (auto& [endpoint, count] : _heartbeat_count) {
    count++;
  }
}

void HeartbeatTracker::handle_heartbeat(uni::net::endpoint_id endpoint_id) {
  auto it = _heartbeat_count.find(endpoint_id);
  if (it == _heartbeat_count.end()) {
    _heartbeat_count.insert({endpoint_id, 0});
  } else {
    it->second = 0;
  }
}

std::vector<uni::net::endpoint_id> HeartbeatTracker::alive_endpoints() {
  auto endpoints = std::vector<uni::net::endpoint_id>();
  auto it = _heartbeat_count.begin();
  // Iterate through all heartbeat counts, removing the endpoints that are
  // dead, and populating the `endpoints` variable with the endpoints
  // that are still alive.
  while (it != _heartbeat_count.end()) {
    if (it->second >= HEARTBEAT_FAILURE_COUNT) {
      it = _heartbeat_count.erase(it);
    } else {
      endpoints.push_back(it->first);
      it = std::next(it);
    }
  }
  return endpoints;
}

boost::optional<uni::net::endpoint_id> HeartbeatTracker::leader_endpoint_id() {
  auto endpoints = alive_endpoints();
  if (endpoints.size() > 0) {
    return alive_endpoints()[0];
  }
  return boost::none;
}

void HeartbeatTracker::debug_print() {
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
