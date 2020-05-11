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
  uni::net::Connections& slave_connections,
  uni::async::AsyncScheduler& scheduler,
  std::vector<uni::net::EndpointId> const& config_endpoints,
  std::string ip_string)
  : _random(),
    _timer_scheduler(background_io_context),
    _transaction_manager(
      constants,
      client_connections,
      master_connections,
      slave_connections,
      scheduler,
      _timer_scheduler,
      _random,
      config_endpoints,
      ip_string,
      [this, &constants, &slave_connections, &client_connections](
        uni::slave::TabletId const& tablet_id,
        uni::server::FailureDetector& failure_detector,
        uni::slave::SlaveConfigManager& config_manager
      ) {
        auto min_index = std::distance(
          _participants_per_thread.begin(),
            std::min_element(
              _participants_per_thread.begin(),
              _participants_per_thread.end()));
        _participants_per_thread[min_index]++;
        auto& thread_and_context = _io_contexts[min_index];
        return uni::custom_unique_ptr<uni::slave::TabletParticipant>(
          new uni::slave::TabletParticipant(
            std::make_unique<uni::async::AsyncSchedulerImpl>(thread_and_context->_io_context),
            std::make_unique<uni::async::TimerAsyncSchedulerImpl>(thread_and_context->_io_context),
            std::make_unique<uni::random::RandomImpl>(),
            constants,
            slave_connections,
            client_connections,
            failure_detector,
            config_manager,
            tablet_id
          ), [this, &min_index](uni::slave::TabletParticipant* tp) {
            _participants_per_thread[min_index]--;
            delete tp;
          }
        );
      }
    )
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
