#include "gtest/gtest.h"

#include <async/testing/ClockTesting.h>
#include <async/testing/TimerAsyncSchedulerTesting.h>
#include <logging/log.h>

namespace uni {
namespace testing {
namespace unit {
namespace async {
namespace testing {

class TimerAsyncSchedulerTestingTest
    : public ::testing::Test {
 protected:
  TimerAsyncSchedulerTestingTest():
    _clock(),
    _scheduler(_clock) {}

  uni::async::ClockTesting _clock;
  uni::async::TimerAsyncSchedulerTesting _scheduler;
};

/////////////// TESTS with schedule_once ///////////////

/**
 * Ensure ScheduleOnce works properly.
 */
TEST_F(TimerAsyncSchedulerTestingTest, ScheduleOnce) {
  auto i = 0;
  _scheduler.schedule_once([&i](){ i++; }, 10);
  _clock.increment_time(9);
  EXPECT_EQ(i, 0) << "The callback should not have run.";
  _clock.increment_time(1);
  EXPECT_EQ(i, 1)
    << "The callback should have run when the clock reached the scheduled time.";
  _clock.increment_time(10);
  EXPECT_EQ(i, 1)
    << "The callback should not have run again afterwards for any reason.";
}

/**
 * Ensure that when two callbacks are scheduled at the same time, the one scheduler
 * afterwards is correspondingly executed afterwards.
 */
TEST_F(TimerAsyncSchedulerTestingTest, ScheduleOnceOverlapping) {
  auto i = 0;
  _scheduler.schedule_once([&i](){ i++; }, 10);
  _clock.increment_time(5);
  _scheduler.schedule_once([&i](){
    EXPECT_EQ(i, 1) << "First callback should have been called before the second";
    i++;
  }, 5);
  _clock.increment_time(5);
  EXPECT_EQ(i, 2) << "Both callbacks should have been called.";
}

/**
 * Ensure that when schedule_once is called with wait time 0 that it doesn't
 * execute immediately. Rather, it waits for an increment in the clock.
 */
TEST_F(TimerAsyncSchedulerTestingTest, ScheduleOnceZeroWait) {
  auto i = 0;
  _scheduler.schedule_once([&i](){ i++; }, 0);
  EXPECT_EQ(i, 0) << "Callback should not have been called without incrementing the clock";
  _clock.increment_time(1);
  EXPECT_EQ(i, 1) << "Callback should have been called";
}

/////////////// TESTS with schedule_once with task chaining ///////////////

/**
 * Ensures that when a task schedules another task, the second task is scheduled from
 * the time the scheduling task is scheduled to run (meaning it's independent of 
 * the increment time of the clock).
 */
TEST_F(TimerAsyncSchedulerTestingTest, ScheduleWithChaining) {
  auto i = 0;
  _scheduler.schedule_once([&i, this](){
    _scheduler.schedule_once([&i](){
      i++;
    }, 5);
    i++;
  }, 5);
  EXPECT_EQ(i, 0) << "Callback should not have been called without incrementing the clock";
  _clock.increment_time(9);
  EXPECT_EQ(i, 1) << "First callback should have been called";
  _clock.increment_time(1);
  EXPECT_EQ(i, 2) << "Second callback should have been called";
}

/////////////// TESTS with schedule_repeated ///////////////

/**
 * Ensure schedule_repeated works properly.
 */
TEST_F(TimerAsyncSchedulerTestingTest, ScheduleRepeated) {
  auto i = 0;
  _scheduler.schedule_repeated([&i](){ i++; }, 10);
  _clock.increment_time(9);
  EXPECT_EQ(i, 0) << "The callback should not have run.";
  _clock.increment_time(1);
  EXPECT_EQ(i, 1)
    << "The callback should have run when the clock reached the scheduled time.";
  _clock.increment_time(9);
  EXPECT_EQ(i, 1) << "The callback should not have run.";
  _clock.increment_time(1);
  EXPECT_EQ(i, 2)
    << "The callback should have run a second time when the clock reached the scheduled time.";
}

/////////////// TESTS with schedule_repeated_finite ///////////////

/**
 * Ensure schedule_repeated_finite works properly.
 */
TEST_F(TimerAsyncSchedulerTestingTest, ScheduleRepeatedFinite) {
  auto i = 0;
  _scheduler.schedule_repeated_finite([&i](){ i++; }, 10, 2);
  _clock.increment_time(9);
  EXPECT_EQ(i, 0) << "The callback should not have run.";
  _clock.increment_time(1);
  EXPECT_EQ(i, 1)
    << "The callback should have run when the clock reached the scheduled time.";
  _clock.increment_time(9);
  EXPECT_EQ(i, 1) << "The callback should not have run.";
  _clock.increment_time(1);
  EXPECT_EQ(i, 2)
    << "The callback should have run a second time when the clock reached the scheduled time.";
  _clock.increment_time(10);
  EXPECT_EQ(i, 2) << "The callback should never run again.";
}

} // namespace testing
} // namespace async
} // namespace unit
} // namespace testing
} // namespace uni