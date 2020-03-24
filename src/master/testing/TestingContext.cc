#include "TestingContext.h"

#include <functional>

#include <master/functors.h>

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
