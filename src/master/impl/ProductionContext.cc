#include "ProductionContext.h"

namespace uni {
namespace master {

ProductionContext::ProductionContext(
  boost::asio::io_context& background_io_context,
  uni::constants::Constants const& constants,
  uni::net::Connections& client_connections,
  uni::net::Connections& slave_connections,
  uni::net::Connections& connections,
  uni::async::AsyncScheduler& scheduler,
  std::vector<uni::net::EndpointId>& config_endpoints,
  std::vector<uni::net::EndpointId>& slave_endpoints)
  : _random(),
    _timer_scheduler(background_io_context),
    _master(
      constants,
      client_connections,
      slave_connections,
      connections,
      _timer_scheduler,
      scheduler,
      _random,
      slave_endpoints,
      config_endpoints) {}

} // namespace master
} // namespace uni
