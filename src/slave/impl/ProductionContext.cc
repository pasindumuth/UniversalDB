#include "ProductionContext.h"

#include <net/IncomingMessage.h>
#include <slave/functors.h>

namespace uni {
namespace slave {

ProductionContext::ThreadAndContext::ThreadAndContext()
  : _io_context(),
    _thread([this] {
      auto work_guard = boost::asio::make_work_guard(_io_context);
      _io_context.run();
    }) {}

ProductionContext::ProductionContext(
  boost::asio::io_context& background_io_context,
  uni::constants::Constants const& constants,
  uni::net::Connections& client_connections,
  uni::net::Connections& master_connections,
  uni::net::Connections& connections,
  uni::async::AsyncSchedulerImpl& scheduler)
  : _random(),
    _timer_scheduler(background_io_context),
    _async_queue(_timer_scheduler),
    _heartbeat_tracker(),
    _failure_detector(
      _heartbeat_tracker,
      connections,
      _timer_scheduler,
      uni::slave::GetEndpoints(_config_manager)),
    _paxos_log(),
    _multipaxos_handler(
      _paxos_log,
      [this, &constants, &connections](uni::paxos::index_t index) {
        return uni::paxos::SinglePaxosHandler(
          constants,
          connections,
          _paxos_log,
          _random,
          index,
          uni::slave::GetEndpoints(_config_manager),
          uni::slave::SendPaxos());
      }),
    _log_syncer(
      constants,
      connections,
      _timer_scheduler,
      _paxos_log,
      uni::slave::GetEndpoints(_config_manager),
      uni::slave::SendSync()),
    _config_manager(
      _async_queue,
      master_connections,
      connections,
      _multipaxos_handler,
      _paxos_log),
    _key_space_manager(
      _async_queue,
      master_connections,
      _multipaxos_handler,
      _paxos_log),
    _slave_handler(
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
              return std::make_unique<uni::async::AsyncSchedulerImpl>(thread_and_context->_io_context);
            },
            std::make_unique<uni::random::RandomImpl>(),
            constants,
            connections,
            client_connections,
            _timer_scheduler,
            _failure_detector,
            _config_manager,
            tablet_id
          ), [this, &min_index](uni::slave::TabletParticipant* tp) {
            _participants_per_thread[min_index]--;
            delete tp;
          }
        );
      },
      _heartbeat_tracker,
      _log_syncer,
      _key_space_manager,
      _multipaxos_handler)
{
  // We subtract 2 from the number of supported threads to account for the
  // LTM thread and background thread are started up manually.
  auto num_supported_threads = std::thread::hardware_concurrency() - 2; 
  for (auto i = 0; i < num_supported_threads; i++) {
    _io_contexts.push_back(std::make_unique<ThreadAndContext>());
    _participants_per_thread.push_back(0);
  }
  scheduler.set_callback([this](uni::net::IncomingMessage message){
    _slave_handler.handle(message);
  });
}

} // namespace slave
} // namespace uni
