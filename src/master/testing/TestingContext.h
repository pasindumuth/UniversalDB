#ifndef UNI_MASTER_TESTINGCONTEXT_H
#define UNI_MASTER_TESTINGCONTEXT_H

#include <functional>
#include <string>

#include <async/testing/AsyncSchedulerTesting.h>
#include <async/testing/ClockTesting.h>
#include <async/testing/TimerAsyncSchedulerTesting.h>
#include <constants/constants.h>
#include <common/common.h>
#include <net/Connections.h>
#include <net/EndpointId.h>
#include <random/testing/RandomTesting.h>
#include <master/DataMaster.h>

namespace uni {
namespace master {

struct TestingContext {
  TestingContext(
    uni::constants::Constants const& constants,
    std::vector<uni::net::EndpointId>& config_endpoints,
    std::vector<uni::net::EndpointId>& slave_endpoints,
    std::string ip_string,
    unsigned random_seed);

  // Primitives
  std::string _ip_string;

  // Helper classes
  uni::random::RandomTesting _random;

  // Singletons
  uni::async::AsyncSchedulerTesting _scheduler;
  uni::net::Connections _client_connections;
  uni::net::Connections _slave_connections;
  uni::net::Connections _connections;
  uni::async::ClockTesting _clock;
  uni::async::TimerAsyncSchedulerTesting _timer_scheduler;
  uni::master::DataMaster _master;
};

} // namespace master
} // namespace uni

#endif // UNI_MASTER_TESTINGCONTEXT_H
