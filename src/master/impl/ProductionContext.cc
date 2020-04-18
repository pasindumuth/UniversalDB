#include "ProductionContext.h"

#include <master/functors.h>
#include <net/IncomingMessage.h>
#include <server/SlaveGroupId.h>

namespace uni {
namespace master {

ProductionContext::ProductionContext(
  boost::asio::io_context& background_io_context,
  uni::constants::Constants const& constants,
  uni::net::Connections& client_connections,
  uni::net::Connections& slave_connections,
  uni::net::Connections& connections,
  uni::async::AsyncSchedulerImpl& scheduler)
  : _random(),
    _async_queue_provider([this](){
      return uni::async::AsyncQueue(_timer_scheduler);
    }),
    _timer_scheduler(background_io_context),
    _async_queue(_timer_scheduler),
    _paxos_log(),
    _multipaxos_handler(
      _paxos_log,
      [this, &constants, &connections](uni::paxos::index_t index) {
        return uni::paxos::SinglePaxosHandler(
          constants,
          connections,
          _paxos_log,
          _random,
          index,
          uni::master::GetEndpoints(connections),
          uni::master::SendPaxos());
      }),
    _log_syncer(
      constants,
      connections,
      _timer_scheduler,
      _paxos_log,
      uni::master::GetEndpoints(connections),
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
  auto endpoints = slave_connections.get_all_endpoints();
  _group_config_manager.set_first_config(group_id, endpoints);
  _key_space_manager.set_first_config(group_id);
}

} // namespace master
} // namespace uni
