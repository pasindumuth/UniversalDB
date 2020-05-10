#include "TestingContext.h"

#include <master/functors.h>

namespace uni {
namespace master {

TestingContext::TestingContext(
  uni::constants::Constants const& constants,
  std::vector<uni::net::EndpointId>& config_endpoints,
  std::vector<uni::net::EndpointId>& slave_endpoints,
  std::string ip_string,
  unsigned random_seed)
  : _ip_string(ip_string),
    _random(random_seed),
    _scheduler(),
    _client_connections(_scheduler),
    _slave_connections(_scheduler),
    _connections(_scheduler),
    _clock(),
    _timer_scheduler(_clock),
    _master(
      constants,
      _client_connections,
      _slave_connections,
      _connections,
      _timer_scheduler,
      _scheduler,
      _random,
      slave_endpoints,
      [&config_endpoints](){ return config_endpoints; }) {}

} // namespace master
} // namespace uni
