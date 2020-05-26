#ifndef UNI_PAXOS_PROPOSERQUEUE_H
#define UNI_PAXOS_PROPOSERQUEUE_H

#include <functional>
#include <optional>
#include <queue>

#include <async/TimerAsyncScheduler.h>
#include <common/common.h>
#include <paxos/MultiPaxosHandler.h>
#include <paxos/PaxosLog.h>
#include <paxos/PaxosTypes.h>
#include <proto/paxos.pb.h>

namespace uni {
namespace paxos {

/**
 * @brief A queue of tasks that are run and repeated, based the return value of the task
 * 
 * We can add an arbitrary number of tasks. If the queue is empty, the first task
 * that's added is scheduled to run as soon as possible (but not immediately in the
 * #add_task method call). When a task is run, the return value is used to determine if the task
 * should be run again (when its >= 0), and how far in the future (which is just the return value).
 * If it shouldn't be run again, the task is popped and the next task is scheduled to run as soon
 * as possible (but not immediately in the #add_task method call).
 */
class ProposerQueue {
 public:
  static int const TERMINATE;
  static int const PROPOSAL_WAIT_TIME;
  static int const PROPOSAL_RETRIES;

  /*
   * We specify the representation invariant of this class that must be maintained
   * before and after every method call. If _tasks is nonempty, then the first
   * task returned a non-empty optional when its try_callback was run, and that
   * returned valued is currently in proposal. There is also a job in the
   * TimerAsyncScheduler that was entered with PROPOSAL_WAIT_TIME away to run
   * try_proposal for that task again with one less `tries`.
   *
   * If _tasks is empty, then there are no scheduled jobs or anything.
   */

  ProposerQueue(
    uni::async::TimerAsyncScheduler& timer_scheduler,
    uni::paxos::PaxosLog& paxos_log,
    uni::paxos::MultiPaxosHandler& multipaxos_handler);

  void try_insert(
    std::function<std::optional<proto::paxos::PaxosLogEntry>()> try_callback,
    std::function<void()> success_callback,
    std::function<void()> failure_callback);

 private:
  struct InsertTask {
    std::function<std::optional<proto::paxos::PaxosLogEntry>()> try_callback;
    std::function<void()> success_callback;
    std::function<void()> failure_callback;
  };

  uni::async::TimerAsyncScheduler& _timer_scheduler;
  uni::paxos::PaxosLog& _paxos_log;
  uni::paxos::MultiPaxosHandler& _multipaxos_handler;

  std::queue<InsertTask> _tasks;

  void run_task();

  // This method pops all tasks in the queue that return an empty
  // PaxosLogEntry. If we get to a task that returns a non-empty optional,
  // we return that value (as an optional). Otherwise, we return an empty
  // optional (indicating that the _tasks queue is empty).
  std::optional<proto::paxos::PaxosLogEntry> remove_invalid_tasks();

  void try_proposal(
    InsertTask task,
    uni::paxos::index_t index,
    proto::paxos::PaxosLogEntry entry,
    unsigned tries);
};

} // namespace paxos
} // namespace uni

#endif // UNI_PAXOS_PROPOSERQUEUE_H
