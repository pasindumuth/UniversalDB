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

class TestDriver {
 public:
  void run_test(std::function<void(
      std::vector<std::unique_ptr<uni::async::AsyncSchedulerTesting>>&,
      std::vector<uni::net::ChannelTesting*>&,
      std::vector<std::unique_ptr<uni::paxos::PaxosLog>>&)>);
};

} // testing
} // uni


#endif // UNI_TESTING_TESTDRIVER_H
