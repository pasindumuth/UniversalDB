#include "DataMaster.h"

#include <master/functors.h>
#include <net/IncomingMessage.h>
#include <server/SlaveGroupId.h>

namespace uni {
namespace master {

DataMaster::DataMaster(
  uni::constants::Constants const& constants,
  uni::net::Connections& client_connections,
  uni::net::Connections& slave_connections,
  uni::net::Connections& connections,
  uni::async::TimerAsyncScheduler& timer_scheduler,
  uni::async::AsyncScheduler& scheduler,
  uni::random::Random& random,
  std::vector<uni::net::EndpointId> slave_endpoints,
  std::vector<uni::net::EndpointId> master_endpoints)
  : _async_queue_provider([&timer_scheduler](){
      return uni::async::AsyncQueue(timer_scheduler);
    }),
    _async_queue(timer_scheduler),
    _paxos_log(),
    _paxos_config_manager(
      0,
      master_endpoints,
      _paxos_log),
    _multipaxos_handler(
      _paxos_log,
      [this, &constants, &connections, &random](uni::paxos::index_t index) {
        return uni::paxos::SinglePaxosHandler(
          constants,
          connections,
          _paxos_log,
          random,
          index,
          _paxos_config_manager.config(index),
          uni::master::SendPaxos());
      }),
    _log_syncer(
      constants,
      connections,
      timer_scheduler,
      _paxos_log,
      [this](){ return _paxos_config_manager.latest_config(); },
      uni::master::SendSync()),
    _group_config_manager(
      _async_queue,
      slave_connections,
      _multipaxos_handler,
      _paxos_log),
    _key_space_manager(
      _async_queue,
      _async_queue_provider,
      _group_config_manager,
      slave_connections,
      _multipaxos_handler,
      _paxos_log,
      uni::master::SendFindKeyRangeResponse(client_connections)),
    _master_handler(
      _log_syncer,
      _multipaxos_handler,
      _group_config_manager,
      _key_space_manager)
{
  scheduler.set_callback([this](uni::net::IncomingMessage message){
    _master_handler.handle(message);
  });
  auto group_id = uni::server::SlaveGroupId{ "slave_group_0" };
  _group_config_manager.set_first_config(group_id, slave_endpoints);
  _key_space_manager.set_first_config(group_id);
}

} // namespace master
} // namespace uni
