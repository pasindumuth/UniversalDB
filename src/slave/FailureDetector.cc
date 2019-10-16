#include "FailureDetector.h"

namespace uni {
namespace slave {

FailureDetector::FailureDetector() {}

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
