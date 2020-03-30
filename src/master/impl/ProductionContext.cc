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
  : async_queue_provider([this](){
      return uni::async::AsyncQueue(timer_scheduler);
    }),
    timer_scheduler(background_io_context),
    async_queue(timer_scheduler),
    paxos_log(),
    multipaxos_handler(
      paxos_log,
      [this, &constants, &connections](uni::paxos::index_t index) {
        return uni::paxos::SinglePaxosHandler(
          constants,
          connections,
          paxos_log,
          index,
          uni::master::GetEndpoints(connections),
          uni::master::SendPaxos());
      }),
    log_syncer(
      constants,
      connections,
      timer_scheduler,
      paxos_log,
      uni::master::GetEndpoints(connections),
      uni::master::SendSync()),
    group_config_manager(
      async_queue,
      slave_connections,
      multipaxos_handler,
      paxos_log),
    key_space_manager(
      async_queue,
      async_queue_provider,
      group_config_manager,
      slave_connections,
      multipaxos_handler,
      paxos_log,
      uni::master::SendFindKeyRangeResponse(client_connections)),
    master_handler(
      log_syncer,
      multipaxos_handler,
      group_config_manager,
      key_space_manager)
{
  scheduler.set_callback([this](uni::net::IncomingMessage message){
    master_handler.handle(message);
  });
  auto group_id = uni::server::SlaveGroupId{ "slave_group_0" };
  auto endpoints = slave_connections.get_all_endpoints();
  group_config_manager.set_first_config(group_id, endpoints);
  key_space_manager.set_first_config(group_id);
}

} // namespace master
} // namespace uni
