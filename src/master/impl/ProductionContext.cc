#include "ProductionContext.h"

#include <async/impl/AsyncSchedulerImpl.h>

namespace uni {
namespace master {

ProductionContext::ProductionContext(
  boost::asio::io_context& background_io_context,
  uni::constants::Constants const& constants,
  uni::net::Connections& client_connections,
  uni::net::Connections& slave_connections,
  uni::net::Connections& connections)
  : timer_scheduler(background_io_context),
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
      [](proto::sync::SyncMessage* sync_message){
        auto message_wrapper = proto::message::MessageWrapper();
        auto slave_message = new proto::slave::SlaveMessage;
        slave_message->set_allocated_sync_message(sync_message);
        message_wrapper.set_allocated_slave_message(slave_message);
        return message_wrapper;
      }),
    group_config_manager(
      async_queue,
      connections,
      slave_connections,
      multipaxos_handler,
      paxos_log),
    key_space_manager(
      async_queue,
      connections,
      multipaxos_handler,
      paxos_log),
    master_handler(
      log_syncer,
      multipaxos_handler,
      group_config_manager,
      key_space_manager) {}

} // namespace master
} // namespace uni
