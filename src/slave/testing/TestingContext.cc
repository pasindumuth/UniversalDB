#include "TestingContext.h"

#include <async/impl/AsyncSchedulerImpl.h>

namespace uni {
namespace slave {

TestingContext::TestingContext(
  uni::constants::Constants const& constants,
  std::string ip_string)
  : ip_string(ip_string),
    scheduler(),
    client_connections(scheduler),
    connections(scheduler),
    clock(),
    timer_scheduler(clock),
    heartbeat_tracker(),
    failure_detector(
      heartbeat_tracker,
      connections,
      timer_scheduler),
    paxos_log(),
    proposer_queue(timer_scheduler),
    multipaxos_handler(
      paxos_log,
      [this, &constants](uni::paxos::index_t index) {
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
    slave_handler(
      [this, &constants](uni::slave::TabletId tablet_id) {
        return uni::custom_unique_ptr<uni::slave::TabletParticipant>(
          new uni::slave::TabletParticipant(
            [](){
              return std::make_unique<uni::async::AsyncSchedulerTesting>();
            },
            constants,
            connections,
            client_connections,
            timer_scheduler,
            failure_detector,
            tablet_id
          ), [this](uni::slave::TabletParticipant* tp) {
            delete tp;
          }
        );
      },
      heartbeat_tracker,
      log_syncer)
{
  scheduler.set_callback([this](uni::net::IncomingMessage message){
    slave_handler.handle(message);
  });
}

} // namespace slave
} // namespace uni
