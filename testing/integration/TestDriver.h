#ifndef UNI_TESTING_TESTDRIVER_H
#define UNI_TESTING_TESTDRIVER_H

#include <functional>
#include <memory>
#include <vector>

#include <common/common.h>
#include <constants/constants.h>
#include <master/testing/TestingContext.h>
#include <net/testing/ChannelTesting.h>
#include <slave/testing/TestingContext.h>

namespace uni {
namespace testing {
namespace integration {

struct TestParams {
  uni::constants::Constants const& constants;

  // A single Slave Group
  std::vector<std::unique_ptr<uni::slave::TestingContext>>& slaves;
  std::vector<std::vector<uni::net::ChannelTesting*>>& slave_channels;
  std::vector<uni::net::ChannelTesting*>& slave_nonempty_channels;

  // The Master Group
  std::vector<std::unique_ptr<uni::master::TestingContext>>& masters;
  std::vector<std::vector<uni::net::ChannelTesting*>>& master_channels;
  std::vector<uni::net::ChannelTesting*>& master_nonempty_channels;

  // The connections between Masters and Slaves
  std::vector<std::vector<uni::net::ChannelTesting*>>& master_slave_channels;
  std::vector<uni::net::ChannelTesting*>& master_slave_nonempty_channels;
  std::vector<std::vector<uni::net::ChannelTesting*>>& slave_master_channels;
  std::vector<uni::net::ChannelTesting*>& slave_master_nonempty_channels;
};

using TestFunction = std::function<void(TestParams)>;

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

} // namespace integration
} // namespace testing
} // namespace uni


#endif // UNI_TESTING_TESTDRIVER_H
