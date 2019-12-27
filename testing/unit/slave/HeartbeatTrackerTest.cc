#include "gtest/gtest.h"

#include <map>
#include <vector>

#include <boost/optional.hpp>

#include <common/common.h>
#include <net/endpoint_id.h>
#include <slave/HeartbeatTracker.h>

namespace uni {
namespace testing {
namespace unit {
namespace slave {

using uni::slave::HeartbeatTracker;

class HeartbeatTrackerTest
    : public ::testing::Test,
      public ::uni::slave::HeartbeatTracker {};

/////////////// increment_counts ///////////////

TEST_F(HeartbeatTrackerTest, IncrementHeartbeatCount1) {
  _heartbeat_count = {
    {{ "ip1", 1610 }, 0},
    {{ "ip2", 1610 }, 0},
    {{ "ip3", 1610 }, 0},
  };
  increment_counts();
  auto expected_value = std::map<uni::net::endpoint_id, uint32_t>{
    {{ "ip1", 1610 }, 1},
    {{ "ip2", 1610 }, 1},
    {{ "ip3", 1610 }, 1},
  };
  EXPECT_EQ(_heartbeat_count, expected_value)
    << "Heartbeat count should have incremented by one";
}

/**
 * Ensure that if the heartbeat is at the failure threshold, that
 * incrementing it further will actually increase it.
 */
TEST_F(HeartbeatTrackerTest, IncrementHeartbeatCount2) {
  _heartbeat_count = {
    {{ "ip1", 1610 }, 0},
    {{ "ip2", 1610 }, 2},
    {{ "ip3", 1610 }, HeartbeatTracker::HEARTBEAT_FAILURE_COUNT},
  };
  increment_counts();
  auto expected_value = std::map<uni::net::endpoint_id, uint32_t>{
    {{ "ip1", 1610 }, 1},
    {{ "ip2", 1610 }, 3},
    {{ "ip3", 1610 }, HeartbeatTracker::HEARTBEAT_FAILURE_COUNT + 1},
  };
  EXPECT_EQ(_heartbeat_count, expected_value)
    << "Heartbeat count should have incremented by one";
}

/////////////// handle_heartbeat ///////////////

/**
 * Ensure that if the heartbeat is at the failure threshold, that
 * incrementing it further will actually increase it.
 */
TEST_F(HeartbeatTrackerTest, HandleHeartbeat) {
  _heartbeat_count = {
    {{ "ip1", 1610 }, 0},
    {{ "ip2", 1610 }, 2},
    {{ "ip3", 1610 }, 4},
  };
  handle_heartbeat({ "ip3", 1610 });
  auto expected_value = std::map<uni::net::endpoint_id, uint32_t>{
    {{ "ip1", 1610 }, 0},
    {{ "ip2", 1610 }, 2},
    {{ "ip3", 1610 }, 0},
  };
  EXPECT_EQ(_heartbeat_count, expected_value)
    << "Heartbeat count should have been set to zero";
}

/////////////// handle_heartbeat ///////////////

/**
 * Ensure that if an endpoint is at the failure count, then
 * it's not considered alive. Also, ensure that it's
 * erased from the heartbeat count.
 */
TEST_F(HeartbeatTrackerTest, AliveEndpoints) {
  _heartbeat_count = {
    {{ "ip1", 1610 }, HeartbeatTracker::HEARTBEAT_FAILURE_COUNT},
    {{ "ip2", 1610 }, HeartbeatTracker::HEARTBEAT_FAILURE_COUNT - 1},
    {{ "ip3", 1610 }, 0},
  };
  auto endpoints = alive_endpoints();
  auto expected_heartbeat_count = std::map<uni::net::endpoint_id, uint32_t>{
    {{ "ip2", 1610 }, HeartbeatTracker::HEARTBEAT_FAILURE_COUNT - 1},
    {{ "ip3", 1610 }, 0},
  };
  auto expected_endpoints = std::vector<uni::net::endpoint_id>{
    { "ip2", 1610 },
    { "ip3", 1610 },
  };
  EXPECT_EQ(_heartbeat_count, expected_heartbeat_count)
    << "Heartbeat count should have changed properly";
  EXPECT_EQ(endpoints, expected_endpoints)
    << "The correct set of endpoints should have been returned";
}

/////////////// leader_endpoint_id ///////////////

/**
 * Ensure the right leader is returned when one exists.
 */
TEST_F(HeartbeatTrackerTest, LeaderEndpointPresent) {
  _heartbeat_count = {
    {{ "ip1", 1610 }, 3},
    {{ "ip2", 1610 }, HeartbeatTracker::HEARTBEAT_FAILURE_COUNT - 1},
    {{ "ip3", 1610 }, 0},
  };
  auto leader = leader_endpoint_id();
  auto expected_leader = boost::optional<uni::net::endpoint_id>(
    uni::net::endpoint_id{ "ip1", 1610 });
  EXPECT_EQ(leader, expected_leader)
    << "Leader should be correct";
}

/**
 * Ensure the an empty optional is returned when there is no leader.
 */
TEST_F(HeartbeatTrackerTest, LeaderEndpointNotPresent) {
  _heartbeat_count = {};
  auto leader = leader_endpoint_id();
  auto expected_leader = boost::none;
  EXPECT_EQ(leader, expected_leader)
    << "Leader should not be present";
}


} // namespace slave
} // namespace unit
} // namespace testing
} // namespace uni