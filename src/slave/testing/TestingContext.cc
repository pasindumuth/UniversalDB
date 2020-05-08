#include "TestingContext.h"

#include <slave/functors.h>

namespace uni {
namespace slave {

TestingContext::TestingContext(
  uni::constants::Constants const& constants,
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
    _async_queue(_timer_scheduler),
    _heartbeat_tracker(),
    _failure_detector(
      _heartbeat_tracker,
      _slave_connections,
      _timer_scheduler,
      uni::slave::GetEndpoints(_config_manager)),
    _paxos_log(),
    _multipaxos_handler(
      _paxos_log,    
      [this, &constants](uni::paxos::index_t index) {
        return uni::paxos::SinglePaxosHandler(
          constants,
          _slave_connections,
          _paxos_log,
          _random,
          index,
          uni::slave::GetEndpoints(_config_manager),
          uni::slave::SendPaxos());
      }),
    _log_syncer(
      constants,
      _slave_connections,
      _timer_scheduler,
      _paxos_log,
      uni::slave::GetEndpoints(_config_manager),
      uni::slave::SendSync()),
    _config_manager(
      _async_queue,
      _master_connections,
      _slave_connections,
      _multipaxos_handler,
      _paxos_log,
      uni::net::EndpointId(ip_string, 0)),
    _key_space_manager(
      _async_queue,
      _master_connections,
      _multipaxos_handler,
      _paxos_log,
      _tablet_manager),
    _tablet_manager(
      [this, &constants](uni::slave::TabletId tablet_id) {
        return uni::custom_unique_ptr<uni::slave::TabletParticipant>(
          new uni::slave::TabletParticipant(
            [](){
              return std::make_unique<uni::async::AsyncSchedulerTesting>();
            },
            std::make_unique<uni::random::RandomTesting>(_random.rng()()),
            constants,
            _slave_connections,
            _client_connections,
            _timer_scheduler,
            _failure_detector,
            _config_manager,
            tablet_id
          ), [this](uni::slave::TabletParticipant* tp) {
            delete tp;
          }
        );
      },
      _client_connections,
      uni::slave::ClientRespond(_client_connections)
    ),
    _slave_handler(
      _tablet_manager,
      _heartbeat_tracker,
      _log_syncer,
      _key_space_manager,
      _multipaxos_handler)
{
  _scheduler.set_callback([this](uni::net::IncomingMessage message){
    _slave_handler.handle(message);
  });
}

} // namespace slave
} // namespace uni
