#include "gtest/gtest.h"

#include <common/common.h>
#include <net/endpoint_id.h>
#include <paxos/PaxosTypes.h>
#include <paxos/PaxosLog.h>
#include <slave/LogSyncer.h>

namespace uni {
namespace testing {
namespace unit {
namespace slave {

class LogSyncerTest
    : public ::testing::Test {};

/////////////// build_sync_request ///////////////

TEST_F(LogSyncerTest, BuildSyncRequestTest) {
  auto request = std::unique_ptr<proto::slave::SyncRequest>(
    uni::slave::_inner::build_sync_request({0, 1, 2, 3, 5, 7, 8, 9, 11}));
  EXPECT_EQ(request->missing_indices().size(), 4);
  EXPECT_EQ(request->missing_indices().at(0).start(), 0);
  EXPECT_EQ(request->missing_indices().at(0).end(), 3);
  EXPECT_EQ(request->missing_indices().at(1).start(), 5);
  EXPECT_EQ(request->missing_indices().at(1).end(), 5);
  EXPECT_EQ(request->missing_indices().at(2).start(), 7);
  EXPECT_EQ(request->missing_indices().at(2).end(), 9);
  EXPECT_EQ(request->missing_indices().at(3).start(), 11);
  EXPECT_EQ(request->missing_indices().at(3).end(), 11);
  EXPECT_EQ(request->last_index(), 11);
}

/////////////// build_sync_response ///////////////

TEST_F(LogSyncerTest, BuildSyncResponseShortLogTest) {
  auto request = proto::slave::SyncRequest();
  auto i1 = new proto::slave::SyncRequest_IndexSubArray;
  i1->set_start(0);
  i1->set_end(1);
  request.mutable_missing_indices()->AddAllocated(i1);
  auto i2 = new proto::slave::SyncRequest_IndexSubArray;
  i2->set_start(3);
  i2->set_end(3);
  request.mutable_missing_indices()->AddAllocated(i2);
  request.set_last_index(3);

  // First, consider a PaxosLog whose last index is less than
  // the last_index of the request
  auto paxos_log = uni::paxos::PaxosLog({
    {0, proto::paxos::PaxosLogEntry()},
    {1, proto::paxos::PaxosLogEntry()},
    {2, proto::paxos::PaxosLogEntry()},
  });
  auto response = std::unique_ptr<proto::slave::SyncResponse>(
    uni::slave::_inner::build_sync_response(paxos_log, request));
  EXPECT_EQ(response->missing_entries().size(), 2);
  EXPECT_EQ(response->missing_entries().at(0).index(), 0);
  EXPECT_EQ(response->missing_entries().at(1).index(), 1);
}

TEST_F(LogSyncerTest, BuildSyncResponseLongLogTest) {
  auto request = proto::slave::SyncRequest();
  auto i1 = new proto::slave::SyncRequest_IndexSubArray;
  i1->set_start(0);
  i1->set_end(1);
  request.mutable_missing_indices()->AddAllocated(i1);
  auto i2 = new proto::slave::SyncRequest_IndexSubArray;
  i2->set_start(3);
  i2->set_end(3);
  request.mutable_missing_indices()->AddAllocated(i2);
  request.set_last_index(3);

  // Then, consider a PaxosLog whose last index is greater than
  // the last_index of the request
  auto paxos_log = uni::paxos::PaxosLog({
    {0, proto::paxos::PaxosLogEntry()},
    {1, proto::paxos::PaxosLogEntry()},
    {2, proto::paxos::PaxosLogEntry()},
    {3, proto::paxos::PaxosLogEntry()},
    {4, proto::paxos::PaxosLogEntry()},
  });
  auto response = std::unique_ptr<proto::slave::SyncResponse>(
    uni::slave::_inner::build_sync_response(paxos_log, request));
  EXPECT_EQ(response->missing_entries().size(), 4);
  EXPECT_EQ(response->missing_entries().at(0).index(), 0);
  EXPECT_EQ(response->missing_entries().at(1).index(), 1);
  EXPECT_EQ(response->missing_entries().at(2).index(), 3);
  EXPECT_EQ(response->missing_entries().at(3).index(), 4);
}

} // namespace slave
} // namespace unit
} // namespace testing
} // namespace uni
