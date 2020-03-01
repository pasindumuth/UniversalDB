#include "ProductionContext.h"

#include <async/impl/AsyncSchedulerImpl.h>

namespace uni {
namespace slave {

ProductionContext::ThreadAndContext::ThreadAndContext()
  : io_context(),
    thread([this] {
      auto work_guard = boost::asio::make_work_guard(io_context);
      io_context.run();
    }) {}

ProductionContext::ProductionContext(
  boost::asio::io_context& background_io_context,
  uni::constants::Constants const& constants,
  uni::net::ConnectionsIn& client_connections_in,
  uni::net::ConnectionsIn& connections_in,
  uni::net::ConnectionsOut& connections_out)
  : timer_scheduler(background_io_context),
    heartbeat_tracker(),
    failure_detector(
      heartbeat_tracker,
      connections_out,
      timer_scheduler),
    paxos_log(),
    multipaxos_handler(
      paxos_log,
      [this, &constants, &connections_out](uni::paxos::index_t index) {
        return uni::paxos::SinglePaxosHandler(
          constants,
          connections_out,
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
      connections_out,
      timer_scheduler,
      paxos_log,
      failure_detector,
      [](proto::sync::SyncMessage* sync_message){
        auto message_wrapper = proto::message::MessageWrapper();
        auto slave_message = new proto::slave::SlaveMessage;
        slave_message->set_allocated_sync_message(sync_message);
        message_wrapper.set_allocated_slave_message(slave_message);
        return message_wrapper;
      }),
    slave_handler(
      [this, &constants, &client_connections_in, &connections_out](uni::slave::TabletId tablet_id) {
        auto min_index = std::distance(
          _participants_per_thread.begin(),
            std::min_element(
              _participants_per_thread.begin(),
              _participants_per_thread.end()));
        _participants_per_thread[min_index]++;
        auto& thread_and_context = _io_contexts[min_index];
        return uni::custom_unique_ptr<uni::slave::TabletParticipant>(
          new uni::slave::TabletParticipant(
            [&thread_and_context](){
              return std::make_unique<uni::async::AsyncSchedulerImpl>(thread_and_context->io_context);
            },
            constants,
            connections_out,
            client_connections_in,
            timer_scheduler,
            failure_detector,
            tablet_id
          ), [this, &min_index](uni::slave::TabletParticipant* tp) {
            _participants_per_thread[min_index]--;
            delete tp;
          }
        );
      },
      heartbeat_tracker,
      log_syncer)
{
  // We subtract 2 from the number of supported threads to account for the
  // LTM thread and background thread are started up manually.
  auto num_supported_threads = std::thread::hardware_concurrency() - 2; 
  for (auto i = 0; i < num_supported_threads; i++) {
    _io_contexts.push_back(std::make_unique<ThreadAndContext>());
    _participants_per_thread.push_back(0);
  }
}

} // namespace slave
} // namespace uni
