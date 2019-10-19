#include "LogSyncer.h"

namespace uni {
namespace slave {

using uni::net::ConnectionsOut;
using uni::async::TimerAsyncScheduler;
using uni::slave::FailureDetector;

LogSyncer::LogSyncer(
    uni::constants::Constants const& constants,
    ConnectionsOut& connections_out,
    TimerAsyncScheduler& timer_scheduler,
    FailureDetector& failure_detector)
      : _constants(constants),
        _connections_out(connections_out),
        _timer_scheduler(timer_scheduler),
        _failure_detector(failure_detector) {}

void LogSyncer::schedule_syncing() {

}

void LogSyncer::handle_sync_request() {

}

void LogSyncer::handle_sync_response() {

}

} // namespace slave
} // namespace uni
