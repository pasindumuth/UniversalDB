#include "ProposerQueue.h"

namespace uni {
namespace paxos {

int const ProposerQueue::TERMINATE = -1;
int const ProposerQueue::PROPOSAL_WAIT_TIME = 100;
int const ProposerQueue::PROPOSAL_RETRIES = 3;

ProposerQueue::ProposerQueue(
  uni::async::TimerAsyncScheduler& timer_scheduler,
  uni::paxos::PaxosLog& paxos_log,
  uni::paxos::MultiPaxosHandler& multipaxos_handler)
  : _timer_scheduler(timer_scheduler),
    _paxos_log(paxos_log),
    _multipaxos_handler(multipaxos_handler) {}

void ProposerQueue::try_insert(
  std::function<std::optional<proto::paxos::PaxosLogEntry>()> try_callback,
  std::function<void()> success_callback,
  std::function<void()> failure_callback
) {
  _tasks.push({
    try_callback,
    success_callback,
    failure_callback
  });
  if (_tasks.size() == 1) {
    run_task();
  }
}

void ProposerQueue::run_task() {
  if (auto entry = remove_invalid_tasks()) {
    // There is a value that should be inserted at this point.
    try_proposal(
      _tasks.front(),
      _paxos_log.next_available_index(),
      entry.value(),
      PROPOSAL_RETRIES);
  }
}

std::optional<proto::paxos::PaxosLogEntry> ProposerQueue::remove_invalid_tasks() {
  while (_tasks.size() > 0) {
    if (auto entry = _tasks.front().try_callback()) {
      return entry;
    } else {
      _tasks.pop();
    }
  }
  return {};
}

void ProposerQueue::try_proposal(
  InsertTask task,
  uni::paxos::index_t index,
  proto::paxos::PaxosLogEntry entry,
  unsigned tries
) {
  if (index < _paxos_log.next_available_index()) {
    // If the next_available_index of the PaxosLog has advanced, this
    // task's success or failure callback will have already been called
    // and would have been popped off of _tasks. Ideally, we should delete
    // try_proposal jobs when their task is deleted from _tasks, but since
    // there is no way to do this, we get here. Thus, we should just exit.
    return;
  }

  if (tries > 0) {
    _multipaxos_handler.propose(entry);
    _timer_scheduler.schedule_once([this, task, index, entry, tries](){
      try_proposal(task, index, entry, tries - 1);
    }, PROPOSAL_WAIT_TIME);
  } else {
    task.failure_callback();
    _tasks.pop();
    run_task();
  }
}

} // namespace paxos
} // namespace uni
