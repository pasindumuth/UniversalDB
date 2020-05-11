#include "LocalTransactionManager.h"

#include <net/EndpointId.h>
#include <slave/functors.h>

namespace uni {
namespace slave {

LocalTransactionManager::LocalTransactionManager(
  uni::constants::Constants const& constants,
  uni::net::Connections& client_connections,
  uni::net::Connections& master_connections,
  uni::net::Connections& slave_connections,
  uni::async::AsyncScheduler& scheduler,
  uni::async::TimerAsyncScheduler& timer_scheduler,
  uni::random::Random& random,
  std::vector<uni::net::EndpointId> const& config_endpoints,
  std::string& ip_string,
  TPProvider tp_provider)
  : _async_queue(timer_scheduler),
    _heartbeat_tracker(),
    _paxos_log(),
    _paxos_config_manager(
      0,
      config_endpoints,
      _paxos_log),
    _multipaxos_handler(
      _paxos_log,
      [this, &constants, &slave_connections, &random](uni::paxos::index_t index) {
        return uni::paxos::SinglePaxosHandler(
          constants,
          slave_connections,
          _paxos_log,
          random,
          index,
          _paxos_config_manager.config(index),
          uni::slave::SendPaxos());
      }),
    _failure_detector(
      _heartbeat_tracker,
      slave_connections,
      timer_scheduler,
      [this](){ return _paxos_config_manager.latest_config(); }),
    _log_syncer(
      constants,
      slave_connections,
      timer_scheduler,
      _paxos_log,
      [this](){ return _paxos_config_manager.latest_config(); },
      uni::slave::SendSync()),
    _config_manager(
      _async_queue,
      master_connections,
      slave_connections,
      _multipaxos_handler,
      _paxos_log,
      uni::net::EndpointId(ip_string, 0)),
    _key_space_manager(
      _async_queue,
      master_connections,
      _multipaxos_handler,
      _paxos_log,
      _tablet_manager),
    _tablet_manager(
      [this, tp_provider](uni::slave::TabletId tablet_id) {
        return tp_provider(
          tablet_id,
          _failure_detector,
          _config_manager
        );
      },
      client_connections,
      uni::slave::ClientRespond(client_connections)
    ),
    _slave_handler(
      _tablet_manager,
      _heartbeat_tracker,
      _log_syncer,
      _key_space_manager,
      _multipaxos_handler)
{
  scheduler.set_callback([this](uni::net::IncomingMessage message){
    _slave_handler.handle(message);
  });
}

} // namespace slave
} // namespace uni
