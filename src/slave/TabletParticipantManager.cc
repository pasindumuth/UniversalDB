#include "TabletParticipantManager.h"

namespace uni {
namespace slave {

TabletParticipantManager::TabletParticipantManager(
  uni::slave::ThreadPool& thread_pool,
  uni::net::ConnectionsIn& client_connections_in,
  uni::net::ConnectionsOut& connections_out,
  uni::slave::FailureDetector& failure_detector,
  uni::slave::HeartbeatTracker& heartbeat_tracker,
  uni::slave::LogSyncer& log_syncer,
  uni::constants::Constants const& constants,
  uni::async::TimerAsyncScheduler& timer_scheduler)
  : _thread_pool(thread_pool),
    _client_connections_in(client_connections_in),
    _connections_out(connections_out),
    _failure_detector(failure_detector),
    _heartbeat_tracker(heartbeat_tracker),
    _log_syncer(log_syncer),
    _constants(constants),
    _timer_scheduler(timer_scheduler)
    // _participants_per_thread(thread_pool.io_contexts.size(), 0)
{
  for (auto i = 0; i < thread_pool.io_contexts.size(); i++) {
    _participants_per_thread.push_back(0);
  }
}

std::unique_ptr<uni::slave::TabletParticipant>& TabletParticipantManager::getTablet(TabletId tablet_id) {
  if (_tp_map.find(tablet_id) == _tp_map.end()) {
    // We have to add a Tablet with the given tablet_id
    // We start by finding the most suitable io_context and thread to 
    // assign the Tablet to.
    auto min_index = std::distance(
      _participants_per_thread.begin(),
      std::min_element(
        _participants_per_thread.begin(),
        _participants_per_thread.end()));
    _participants_per_thread[min_index]++;
    auto& thread_and_context = _thread_pool.io_contexts[min_index];
    _tp_map.insert({
      tablet_id,
      std::make_unique<uni::slave::TabletParticipant>(
        thread_and_context->io_context,
        thread_and_context->thread,
        _constants,
        _connections_out,
        _client_connections_in,
        _timer_scheduler,
        _failure_detector,
        tablet_id
      )
    });
  }
  return _tp_map[tablet_id];
}

} // namespace slave
} // namespace uni
