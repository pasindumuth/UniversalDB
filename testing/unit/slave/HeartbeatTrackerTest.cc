#include "gtest/gtest.h"

#include <common/common.h>
#include <net/endpoint_id.h>
#include <slave/HeartbeatTracker.h>

namespace uni {
namespace testing {
namespace unit {
namespace slave {

class HeartbeatTrackerTest
    : public ::testing::Test {};

/////////////// increment_counts ///////////////

TEST_F(HeartbeatTrackerTest, IncrementHeartbeatCount1) {
  auto heartbeat_count = std::map<uni::net::endpoint_id, uint32_t>{
    {{ "ip1", 0 }, 0},
    {{ "ip2", 0 }, 0},
    {{ "ip3", 0 }, 0},
  };
  uni::slave::_inner::increment_counts(heartbeat_count);
  auto expected_value = std::map<uni::net::endpoint_id, uint32_t>{
    {{ "ip1", 0 }, 1},
    {{ "ip2", 0 }, 1},
    {{ "ip3", 0 }, 1},
  };
  EXPECT_EQ(heartbeat_count, expected_value)
    << "Heartbeat count should have incremented by one";
}

/**
 * Ensure that if the heartbeat is at the failure threshold, that
 * incrementing it further will actually increase it.
 */
TEST_F(HeartbeatTrackerTest, IncrementHeartbeatCount2) {
  auto heartbeat_count = std::map<uni::net::endpoint_id, uint32_t>{
    {{ "ip1", 0 }, 0},
    {{ "ip2", 0 }, 2},
    {{ "ip3", 0 }, uni::slave::HeartbeatTracker::HEARTBEAT_FAILURE_COUNT},
  };
  uni::slave::_inner::increment_counts(heartbeat_count);
  auto expected_value = std::map<uni::net::endpoint_id, uint32_t>{
    {{ "ip1", 0 }, 1},
    {{ "ip2", 0 }, 3},
    {{ "ip3", 0 }, uni::slave::HeartbeatTracker::HEARTBEAT_FAILURE_COUNT + 1},
  };
  EXPECT_EQ(heartbeat_count, expected_value)
    << "Heartbeat count should have incremented by one";
}

/////////////// handle_heartbeat ///////////////

/**
 * Ensure that if the heartbeat is at the failure threshold, that
 * incrementing it further will actually increase it.
 */
TEST_F(HeartbeatTrackerTest, HandleHeartbeat) {
  auto heartbeat_count = std::map<uni::net::endpoint_id, uint32_t>{
    {{ "ip1", 0 }, 0},
    {{ "ip2", 0 }, 2},
    {{ "ip3", 0 }, 4},
  };
  uni::slave::_inner::handle_heartbeat(heartbeat_count, { "ip3", 0 });
  auto expected_value = std::map<uni::net::endpoint_id, uint32_t>{
    {{ "ip1", 0 }, 0},
    {{ "ip2", 0 }, 2},
    {{ "ip3", 0 }, 0},
  };
  EXPECT_EQ(heartbeat_count, expected_value)
    << "Heartbeat count should have been set to zero";
}

/////////////// handle_heartbeat ///////////////

/**
 * Ensure that if an endpoint is at the failure count, then
 * it's not considered alive. Also, ensure that it's
 * erased from the heartbeat count.
 */
TEST_F(HeartbeatTrackerTest, AliveEndpoints) {
  auto heartbeat_count = std::map<uni::net::endpoint_id, uint32_t>{
    {{ "ip1", 0 }, uni::slave::HeartbeatTracker::HEARTBEAT_FAILURE_COUNT},
    {{ "ip2", 0 }, uni::slave::HeartbeatTracker::HEARTBEAT_FAILURE_COUNT - 1},
    {{ "ip3", 0 }, 0},
  };
  auto endpoints = uni::slave::_inner::alive_endpoints(heartbeat_count);
  auto expected_heartbeat_count = std::map<uni::net::endpoint_id, uint32_t>{
    {{ "ip2", 0 }, uni::slave::HeartbeatTracker::HEARTBEAT_FAILURE_COUNT - 1},
    {{ "ip3", 0 }, 0},
  };
  auto expected_endpoints = std::vector<uni::net::endpoint_id>{
    { "ip2", 0 },
    { "ip3", 0 },
  };
  EXPECT_EQ(heartbeat_count, expected_heartbeat_count)
    << "Heartbeat count should have changed properly";
  EXPECT_EQ(endpoints, expected_endpoints)
    << "The correct set of endpoints should have been returned";
}

/////////////// leader_endpoint_id ///////////////

/**
 * Ensure the right leader is returned when one exists.
 */
TEST_F(HeartbeatTrackerTest, LeaderEndpointPresent) {
  auto heartbeat_count = std::map<uni::net::endpoint_id, uint32_t>{
    {{ "ip1", 0 }, 3},
    {{ "ip2", 0 }, uni::slave::HeartbeatTracker::HEARTBEAT_FAILURE_COUNT - 1},
    {{ "ip3", 0 }, 0},
  };
  auto leader = uni::slave::_inner::leader_endpoint_id(heartbeat_count);
  auto expected_leader = boost::optional<uni::net::endpoint_id>(
    uni::net::endpoint_id{ "ip1", 0 });
  EXPECT_EQ(leader, expected_leader)
    << "Leader should be correct";
}

/**
 * Ensure the an empty optional is returned when there is no leader.
 */
TEST_F(HeartbeatTrackerTest, LeaderEndpointNotPresent) {
  auto heartbeat_count = std::map<uni::net::endpoint_id, uint32_t>{};
  auto leader = uni::slave::_inner::leader_endpoint_id(heartbeat_count);
  auto expected_leader = boost::none;
  EXPECT_EQ(leader, expected_leader)
    << "Leader should not be present";
}

/////////////// integration test ///////////////

/**
 * Run a basic test on all methods in Heartbeat Tracker.
 */
TEST_F(HeartbeatTrackerTest, IntegrationTest) {
  auto tracker = uni::slave::HeartbeatTracker();
  tracker.handle_heartbeat({ "ip1", 0 });
  tracker.handle_heartbeat({ "ip2", 0 });
  for (auto i = 0; i < uni::slave::HeartbeatTracker::HEARTBEAT_FAILURE_COUNT; i++) {
      tracker.increment_counts();
  }
  tracker.handle_heartbeat({ "ip3", 0 });
  tracker.increment_counts();
  auto endpoints = tracker.alive_endpoints();
  EXPECT_EQ(endpoints.size(), 1) << "There should only be one alive endpoint";
  auto leader = tracker.leader_endpoint_id();
  EXPECT_EQ(leader, boost::optional<uni::net::endpoint_id>({ "ip3", 0 }))
    << "Leader should not be present";
}

} // namespace slave
} // namespace unit
} // namespace testing
} // namespace uni
