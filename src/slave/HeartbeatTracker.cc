#include "HeartbeatTracker.h"

#include <sstream>

#include <common/common.h>

namespace uni {
namespace slave {

uint32_t const HeartbeatTracker::HEARTBEAT_FAILURE_COUNT = 4;
uint32_t const HeartbeatTracker::HEARTBEAT_SEND_PERIOD = 1000;

HeartbeatTracker::HeartbeatTracker() {}

void HeartbeatTracker::increment_counts() {
  _inner::increment_counts(_heartbeat_count);
}

void HeartbeatTracker::handle_heartbeat(uni::net::endpoint_id endpoint_id) {
  _inner::handle_heartbeat(_heartbeat_count, endpoint_id);
}

std::vector<uni::net::endpoint_id> HeartbeatTracker::alive_endpoints() {
  return _inner::alive_endpoints(_heartbeat_count);
}

boost::optional<uni::net::endpoint_id> HeartbeatTracker::leader_endpoint_id() {
  return _inner::leader_endpoint_id(_heartbeat_count);
}

std::string HeartbeatTracker::debug_string() {
  return _inner::debug_string(_heartbeat_count);
}

///////////////////////////// Inner Implementation Details /////////////////////////

namespace _inner {
  
// Increment the heartbeat_count of each endpoint.
void increment_counts(
  std::map<uni::net::endpoint_id, uint32_t>& heartbeat_count
) {
  for (auto& [endpoint, count] : heartbeat_count) {
    count++;
  }
}

// If the endpoint_id isn't in the heartbeat_count, add it in and set its value to
// 0. Otherwise, reset its value to 0.
void handle_heartbeat(
  std::map<uni::net::endpoint_id, uint32_t>& heartbeat_count,
  uni::net::endpoint_id endpoint_id
) {
  auto it = heartbeat_count.find(endpoint_id);
  if (it == heartbeat_count.end()) {
    heartbeat_count.insert({endpoint_id, 0});
  } else {
    it->second = 0;
  }
}

// Return all endpoints with heartbeat_counts < HEARTBEAT_FAILURE_COUNT.
// If there are any endpoints where this doesn't hold, get rid fo them from
// heartbeat_count.
std::vector<uni::net::endpoint_id> alive_endpoints(
  std::map<uni::net::endpoint_id, uint32_t>& heartbeat_count
) {
  auto endpoints = std::vector<uni::net::endpoint_id>();
  auto it = heartbeat_count.begin();
  // Iterate through all heartbeat counts, removing the endpoints that are
  // dead, and populating the `endpoints` variable with the endpoints
  // that are still alive.
  while (it != heartbeat_count.end()) {
    if (it->second >= HeartbeatTracker::HEARTBEAT_FAILURE_COUNT) {
      it = heartbeat_count.erase(it);
    } else {
      endpoints.push_back(it->first);
      it = std::next(it);
    }
  }
  return endpoints;
}

// Call _alive_endpoints, and if there is at least one alive endpoint, return
// the first one. This is the leader endpoint.
boost::optional<uni::net::endpoint_id> leader_endpoint_id(
  std::map<uni::net::endpoint_id, uint32_t>& heartbeat_count
) {
  auto endpoints = alive_endpoints(heartbeat_count);
  if (endpoints.size() > 0) {
    return endpoints[0];
  }
  return boost::none;
}

// Creates a debug string with the heartbeat count
std::string debug_string(std::map<uni::net::endpoint_id, uint32_t>& heartbeat_count) {
  auto ss = std::stringstream();
  ss << "HeartbeatTracker: {" << std::endl;
  for (auto const& [endpoint, count] : heartbeat_count) {
    ss << "  {" << endpoint.ip_string << ", " << std::to_string(count) << "}" << std::endl;
  }
  ss << "}" << std::endl;
  return ss.str();
}

} // namespace _inner
} // namespace slave
} // namespace uni
