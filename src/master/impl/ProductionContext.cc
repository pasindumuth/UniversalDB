#include "ProductionContext.h"

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
  std::vector<uni::net::EndpointId>& slave_endpoints,
  std::vector<uni::net::EndpointId>& config_endpoints,
  uni::async::AsyncSchedulerImpl& scheduler)
  : timer_scheduler(background_io_context),
    async_queue(timer_scheduler),
    paxos_log(),
    multipaxos_handler(
      paxos_log,
      [this, &constants, &connections, &config_endpoints](uni::paxos::index_t index) {
        return uni::paxos::SinglePaxosHandler(
          constants,
          connections,
          paxos_log,
          index,
          [&config_endpoints](){
            return config_endpoints;
          },
          [](proto::paxos::PaxosMessage* paxos_message){
            auto message_wrapper = proto::message::MessageWrapper();
            auto slave_message = new proto::slave::SlaveMessage;
            slave_message->set_allocated_paxos_message(paxos_message);
            message_wrapper.set_allocated_slave_message(slave_message);
            return message_wrapper;
          });
      }),
    log_syncer(
      constants,
      connections,
      timer_scheduler,
      paxos_log,
      [&config_endpoints](){
        return config_endpoints;
      },
      [](proto::sync::SyncMessage* sync_message){
        auto message_wrapper = proto::message::MessageWrapper();
        auto slave_message = new proto::slave::SlaveMessage;
        slave_message->set_allocated_sync_message(sync_message);
        message_wrapper.set_allocated_slave_message(slave_message);
        return message_wrapper;
      }),
    group_config_manager(
      async_queue,
      slave_connections,
      multipaxos_handler,
      paxos_log),
    key_space_manager(
      async_queue,
      group_config_manager,
      slave_connections,
      multipaxos_handler,
      paxos_log),
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
  group_config_manager.set_first_config(group_id, slave_endpoints);
  key_space_manager.set_first_config(group_id);
}

} // namespace master
} // namespace uni
