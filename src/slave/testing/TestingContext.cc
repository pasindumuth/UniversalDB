#include "TestingContext.h"

#include <slave/functors.h>

namespace uni {
namespace slave {

TestingContext::TestingContext(
  uni::constants::Constants const& constants,
  std::vector<uni::net::EndpointId> const& config_endpoints,
  std::string ip_string,
  unsigned random_seed)
  : _ip_string(ip_string),
    _random(random_seed),
    _scheduler(),
    _client_connections(_scheduler),
    _master_connections(_scheduler),
    _slave_connections(_scheduler),
    _clock(),
    _timer_scheduler(_clock),
    _transaction_manager(
      constants,
      _client_connections,
      _master_connections,
      _slave_connections,
      _scheduler,
      _timer_scheduler,
      _random,
      config_endpoints,
      ip_string,
      [this, &constants](
        uni::slave::TabletId const& tablet_id,
        uni::server::FailureDetector& failure_detector,
        uni::slave::SlaveConfigManager& config_manager
      ) {
        return uni::custom_unique_ptr<uni::slave::TabletParticipant>(
          new uni::slave::TabletParticipant(
            std::make_unique<uni::async::AsyncSchedulerTesting>(),
            std::make_unique<uni::async::TimerAsyncSchedulerTesting>(_clock),
            std::make_unique<uni::random::RandomTesting>(_random.rng()()),
            constants,
            _slave_connections,
            _client_connections,
            failure_detector,
            config_manager,
            tablet_id
          ), [this](uni::slave::TabletParticipant* tp) {
            delete tp;
          }
        );
      }
    ) {}

} // namespace slave
} // namespace uni
