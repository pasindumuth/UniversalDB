#include <assert/assert.h>
#include <logging/log.h>
#include <testing/TestDriver.h>
#include <testing/Tests.h>

/*
 * Our testing scheme is as follows. We mock out Channel and AsyncScheduler such that
 * the network becomes under the control of the Simulation thread.
 *
 * In our simulation, we have 5 Universal Slave servers. We index each server with an integer.
 * Whenever we have arrays, the position of the elements are what we use to determine the
 * Slave it's associated with.
 */
int main(int argc, char* argv[]) {
  auto test_driver = uni::testing::TestDriver();
  auto tests = uni::testing::Tests();
  uni::logging::get_log_level() = uni::logging::Level::FATAL;
  std::cout << uni::logging::get_log_level() << std::endl;
  try {
    test_driver.run_test(tests.test1());
    test_driver.run_test(tests.test2());
    test_driver.run_test(tests.test3());
    test_driver.run_test(tests.test4());
  } catch (uni::assert::UniversalException e) {
    std::cout << e.what() << std::endl;
  }
}
