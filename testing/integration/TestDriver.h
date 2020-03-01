#ifndef UNI_TESTING_TESTDRIVER_H
#define UNI_TESTING_TESTDRIVER_H

#include <functional>
#include <memory>
#include <vector>

#include <common/common.h>
#include <constants/constants.h>
#include <net/testing/ChannelTesting.h>
#include <slave/TestingContext.h>

namespace uni {
namespace testing {
namespace integration {

using TestFunction = std::function<void(
    uni::constants::Constants const&,
    std::vector<std::unique_ptr<uni::slave::TestingContext>>&,
    std::vector<std::vector<uni::net::ChannelTesting*>>&,
    std::vector<uni::net::ChannelTesting*>&)>;

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

} // integration
} // testing
} // uni


#endif // UNI_TESTING_TESTDRIVER_H
