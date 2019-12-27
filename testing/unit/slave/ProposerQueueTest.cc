#include "gtest/gtest.h"

#include <async/testing/ClockTesting.h>
#include <async/testing/TimerAsyncSchedulerTesting.h>
#include <common/common.h>
#include <slave/ProposerQueue.h>

namespace uni {
namespace testing {
namespace unit {
namespace slave {

class ProposerQueueTest
    : public ::testing::Test {
 protected:
  ProposerQueueTest()
    : _clock(),
      _scheduler(_clock),
      _queue(_scheduler) {}
  uni::async::ClockTesting _clock;
  uni::async::TimerAsyncSchedulerTesting _scheduler;
  uni::slave::ProposerQueue _queue;
};

/**
 * Ensures that when a task is scheduled that returns different values on
 * different runs, that as the time is incremented, the task is repeated as expected.
 */
TEST_F(ProposerQueueTest, SingleTaskTest) {
  auto i = 3;
  auto j = 0;
  _queue.add_task([&i, &j](){
    i--;
    j++;
    return i;
  });
  EXPECT_EQ(j, 0) << "Task shouldn't have been run without incrementing the clock";
  _clock.increment_time(1);
  EXPECT_EQ(j, 1) << "Task should have run soon, since it was the first in the queue";
  _clock.increment_time(1);
  EXPECT_EQ(j, 1) << "Task should not have run because the time didn't increment enough";
  _clock.increment_time(1);
  EXPECT_EQ(j, 2) << "Task should have run";
  _clock.increment_time(1);
  EXPECT_EQ(j, 4)
    << "Task should have run twice. Once because it was the right time, and another "
       "because it was scheduled to run at time 0.";
  EXPECT_TRUE(_queue.empty()) << "The queue should be empty";
}

/**
 * Ensures that when a task is scheduled that returns different values on
 * different runs, that as the time is incremented, the task is repeated as expected.
 */
TEST_F(ProposerQueueTest, MultileTasksTest) {
  auto i = 3;
  auto j = 3;
  auto k = 0;
  _queue.add_task([&i, &k](){
    i--;
    k++;
    return i;
  });
  _queue.add_task([&j, &k](){
    j--;
    k++;
    return j;
  });
  _clock.increment_time(4);
  EXPECT_EQ(k, 5)
    << "The first task should have completely run, and the second task should have"
       "run once run and finished";
  EXPECT_EQ(i, -1) << "The first task should have completely run and finished";
  _clock.increment_time(3);
  EXPECT_EQ(k, 8) << "The both tasks should have completed";
  EXPECT_EQ(j, -1) << "The second should have completely run and finished";
}

} // namespace slave
} // namespace unit
} // namespace testing
} // namespace uni