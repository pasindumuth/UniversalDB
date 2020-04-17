#include "TestingContext.h"

#include <slave/functors.h>

namespace uni {
namespace slave {

TestingContext::TestingContext(
  uni::constants::Constants const& constants,
  std::string ip,
  unsigned random_seed)
  : ip_string(ip),
    random(random_seed),
    scheduler(),
    client_connections(scheduler),
    master_connections(scheduler),
    connections(scheduler),
    clock(),
    timer_scheduler(clock),
    heartbeat_tracker(),
    failure_detector(
      heartbeat_tracker,
      connections,
      timer_scheduler,
      uni::slave::GetEndpoints(config_manager)),
    paxos_log(),
    async_queue(timer_scheduler),
    multipaxos_handler(
      paxos_log,    
      [this, &constants](uni::paxos::index_t index) {
        return uni::paxos::SinglePaxosHandler(
          constants,
          connections,
          paxos_log,
          random,
          index,
          uni::slave::GetEndpoints(config_manager),
          uni::slave::SendPaxos());
      }),
    log_syncer(
      constants,
      connections,
      timer_scheduler,
      paxos_log,
      uni::slave::GetEndpoints(config_manager),
      uni::slave::SendSync()),
    config_manager(
      async_queue,
      master_connections,
      connections,
      multipaxos_handler,
      paxos_log),
    key_space_manager(
      async_queue,
      master_connections,
      multipaxos_handler,
      paxos_log),
    slave_handler(
      [this, &constants](uni::slave::TabletId tablet_id) {
        return uni::custom_unique_ptr<uni::slave::TabletParticipant>(
          new uni::slave::TabletParticipant(
            [](){
              return std::make_unique<uni::async::AsyncSchedulerTesting>();
            },
            std::make_unique<uni::random::RandomTesting>(random.rng()()),
            constants,
            connections,
            client_connections,
            timer_scheduler,
            failure_detector,
            config_manager,
            tablet_id
          ), [this](uni::slave::TabletParticipant* tp) {
            delete tp;
          }
        );
      },
      heartbeat_tracker,
      log_syncer,
      key_space_manager,
      multipaxos_handler)
{
  scheduler.set_callback([this](uni::net::IncomingMessage message){
    slave_handler.handle(message);
  });
}

} // namespace slave
} // namespace uni
