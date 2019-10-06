#ifndef UNI_TESTING_TESTDRIVER_H
#define UNI_TESTING_TESTDRIVER_H

#include <functional>
#include <memory>
#include <vector>

#include <async/testing/AsyncSchedulerTesting.h>
#include <net/testing/ChannelTesting.h>
#include <paxos/PaxosLog.h>

namespace uni {
namespace testing {

using TestFunction = std::function<void(
    std::vector<std::unique_ptr<uni::async::AsyncSchedulerTesting>>&,
    std::vector<uni::net::ChannelTesting*>&,
    std::vector<std::unique_ptr<uni::paxos::PaxosLog>>&)>;

/**
 * Recall the simulation testing method we use for testing Paxos
 * http://localhost:3000/projects/universaldb/simulationtesting. This class
 * is responsible for setting up a Paxos Group in memory, and subsequently
 * running the test provided.
 */
class TestDriver {
 public:
  // Main function for running tests. The test takes in 3 vectors.
  void run_test(TestFunction test);
};

} // testing
} // uni


#endif // UNI_TESTING_TESTDRIVER_H
