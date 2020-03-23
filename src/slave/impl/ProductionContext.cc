#include "ProductionContext.h"

#include <net/IncomingMessage.h>

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
  uni::net::Connections& client_connections,
  uni::net::Connections& master_connections,
  uni::net::Connections& connections,
  uni::async::AsyncSchedulerImpl& scheduler)
  : timer_scheduler(background_io_context),
    async_queue(timer_scheduler),
    heartbeat_tracker(),
    failure_detector(
      heartbeat_tracker,
      connections,
      timer_scheduler,
      [this](){
        return config_manager.config_endpoints();
      }),
    paxos_log(),
    multipaxos_handler(
      paxos_log,
      [this, &constants, &connections](uni::paxos::index_t index) {
        return uni::paxos::SinglePaxosHandler(
          constants,
          connections,
          paxos_log,
          index,
          [this](){
            return config_manager.config_endpoints();
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
      [this](){
        return config_manager.config_endpoints();
      },
      [](proto::sync::SyncMessage* sync_message){
        auto message_wrapper = proto::message::MessageWrapper();
        auto slave_message = new proto::slave::SlaveMessage;
        slave_message->set_allocated_sync_message(sync_message);
        message_wrapper.set_allocated_slave_message(slave_message);
        return message_wrapper;
      }),
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
      [this, &constants, &client_connections, &connections](uni::slave::TabletId tablet_id) {
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
            connections,
            client_connections,
            timer_scheduler,
            failure_detector,
            config_manager,
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
  scheduler.set_callback([this](uni::net::IncomingMessage message){
    slave_handler.handle(message);
  });
}

} // namespace slave
} // namespace uni
