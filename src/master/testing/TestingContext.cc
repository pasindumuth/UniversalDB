#include "TestingContext.h"

#include <functional>

namespace uni {
namespace master {

TestingContext::TestingContext(
  uni::constants::Constants const& constants,
  std::vector<uni::net::EndpointId>& config_endpoints,
  std::string ip_string)
  : ip_string(ip_string),
    scheduler(),
    client_connections(scheduler),
    slave_connections(scheduler),
    connections(scheduler),
    clock(),
    timer_scheduler(clock),
    paxos_log(),
    async_queue(timer_scheduler),
    multipaxos_handler(
      paxos_log,    
      [this, &constants, &config_endpoints](uni::paxos::index_t index) {
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
{}

} // namespace master
} // namespace uni
