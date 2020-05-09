#ifndef UNI_SLAVE_TESTINGCONTEXT_H
#define UNI_SLAVE_TESTINGCONTEXT_H

#include <functional>
#include <memory>
#include <string>

#include <async/testing/AsyncSchedulerTesting.h>
#include <async/testing/ClockTesting.h>
#include <async/testing/TimerAsyncSchedulerTesting.h>
#include <constants/constants.h>
#include <common/common.h>
#include <net/Connections.h>
#include <net/EndpointId.h>
#include <random/testing/RandomTesting.h>
#include <slave/LocalTransactionManager.h>

namespace uni {
namespace slave {

struct TestingContext {
  TestingContext(
    uni::constants::Constants const& constants,
    std::string ip_string,
    unsigned random_seed);

  // Primitives
  std::string _ip_string;

  // Singletons
  uni::random::RandomTesting _random;
  uni::async::AsyncSchedulerTesting _scheduler;
  uni::net::Connections _client_connections;
  uni::net::Connections _master_connections;
  uni::net::Connections _slave_connections;
  uni::async::ClockTesting _clock;
  uni::async::TimerAsyncSchedulerTesting _timer_scheduler;
  uni::slave::LocalTransactionManager _transaction_manager;
};

} // namespace slave
} // namespace uni

#endif // UNI_SLAVE_TESTINGCONTEXT_H
