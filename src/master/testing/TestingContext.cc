#include "TestingContext.h"

#include <master/functors.h>

namespace uni {
namespace master {

TestingContext::TestingContext(
  uni::constants::Constants const& constants,
  std::vector<uni::net::EndpointId>& config_endpoints,
  std::string ip,
  unsigned random_seed)
  : ip_string(ip),
    random(random_seed),
    async_queue_provider([this](){
      return uni::async::AsyncQueue(timer_scheduler);
    }),
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
          random,
          index,
          [&config_endpoints](){ return config_endpoints; },
          uni::master::SendPaxos());
      }),
    log_syncer(
      constants,
      connections,
      timer_scheduler,
      paxos_log,
      [&config_endpoints](){ return config_endpoints; },
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
}

} // namespace master
} // namespace uni
