#include "TestingContext.h"

#include <master/functors.h>

namespace uni {
namespace master {

TestingContext::TestingContext(
  uni::constants::Constants const& constants,
  std::vector<uni::net::EndpointId>& config_endpoints,
  std::string ip_string,
  unsigned random_seed)
  : _ip_string(ip_string),
    _random(random_seed),
    _async_queue_provider([this](){
      return uni::async::AsyncQueue(_timer_scheduler);
    }),
    _scheduler(),
    _client_connections(_scheduler),
    _slave_connections(_scheduler),
    _connections(_scheduler),
    _clock(),
    _timer_scheduler(_clock),
    _paxos_log(),
    _async_queue(_timer_scheduler),
    _multipaxos_handler(
      _paxos_log,    
      [this, &constants, &config_endpoints](uni::paxos::index_t index) {
        return uni::paxos::SinglePaxosHandler(
          constants,
          _connections,
          _paxos_log,
          _random,
          index,
          [&config_endpoints](){ return config_endpoints; },
          uni::master::SendPaxos());
      }),
    _log_syncer(
      constants,
      _connections,
      _timer_scheduler,
      _paxos_log,
      [&config_endpoints](){ return config_endpoints; },
      uni::master::SendSync()),
    _group_config_manager(
      _async_queue,
      _slave_connections,
      _multipaxos_handler,
      _paxos_log),
    _key_space_manager(
      _async_queue,
      _async_queue_provider,
      _group_config_manager,
      _slave_connections,
      _multipaxos_handler,
      _paxos_log,
      uni::master::SendFindKeyRangeResponse(_client_connections)),
    _master_handler(
      _log_syncer,
      _multipaxos_handler,
      _group_config_manager,
      _key_space_manager)
{
  _scheduler.set_callback([this](uni::net::IncomingMessage message){
    _master_handler.handle(message);
  });
}

} // namespace master
} // namespace uni
