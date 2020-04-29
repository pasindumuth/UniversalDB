#ifndef UNI_TESTING_TESTS_H
#define UNI_TESTING_TESTS_H

#include <memory>
#include <vector>

#include <async/testing/ClockTesting.h>
#include <common/common.h>
#include <paxos/PaxosLog.h>
#include <proto/message_client.pb.h>
#include <proto/message.pb.h>
#include <integration/TestDriver.h>

namespace uni {
namespace testing {
namespace integration {

/**
 * This class defines test simulations. In a test simulation, the order in which
 * Paxos messages are delivered and dropped is defined, as well as when client
 * messages are sent to the Paxos Slaves. Ideally, many different simulation
 * scenarios should be here.
 */
class Tests {
 public:
  // This is a test where the sending of client requests, the delivery of Paxos messages,
  // and the dropping of Paxos messages are all interleaved. We drop a quarter of all
  // Paxos messages.
  TestFunction test1();

  // This is a test where the sending of client requests, the delivery of Paxos messages,
  // the dropping of Paxos messages are interleaved, and a node is failed at some
  // random time.
  TestFunction test2();

  // This is a test that tests the FailureDetector by failing a node, waiting some time
  // and seeing if the remaining nodes have marked that node as deleted.
  TestFunction test3();

  // This is a test where PaxosLogs are made to be populated evenly (by partitioning
  // off some of the nodes and populated the other nodes with message), and the LogSyncer
  // is left to do it's job and even out the PaxosLogs across all nodes.
  TestFunction test4();

 private:
  // Creates a MessageWrapper (the top level message that is sent over
  // the network) using data that only constitutes the client message.
  proto::message::MessageWrapper build_client_request(std::string message);

  // The paxos logs among the group of test slaves looks like the following:
  // 
  // slave1:  slave_log  tp_log1  tp_log2  <empty>
  // slave2:  slave_log  <empty>  tp_log2  tp_log3
  //
  // The first index in the return value corresond to the column in the above
  // table, and the second index correspond to a row. The <empty> entries 
  // are the case where a TP exists for one slave but not another
  std::vector<std::vector<boost::optional<uni::paxos::PaxosLog*>>> get_aligned_logs(
    std::vector<std::unique_ptr<uni::slave::TestingContext>>& slaves);

  // Filters out only pointers that are present
  std::vector<uni::paxos::PaxosLog*> get_present_logs(
    std::vector<boost::optional<uni::paxos::PaxosLog*>> logs);

  // Goes throw all corresponding sets of paxos logs across the test slaves and then verifies
  // if they are compatible. If they aren't, then this method returns false.
  bool verify_all_paxos_logs(std::vector<std::unique_ptr<uni::slave::TestingContext>>& slaves);

  // Goes through all paxos logs and makes sure they are compatible. That is,
  // if log index i is populated in logs l1 and l2 with values v1 and v2, then v1 = v2.
  bool verify_paxos_logs(std::vector<uni::paxos::PaxosLog*> paxos_logs);

  void print_paxos_logs(std::vector<std::unique_ptr<uni::slave::TestingContext>>& slaves);

  // Looks at the proposer queues and returns true iff there is a task scheduled in one.
  bool some_async_queue_nonempty(
    std::vector<std::unique_ptr<uni::slave::TestingContext>>& slaves);

  // Create a client connection to a DM, send it a FindKeySpace request with the provided
  // database_id and table_id, and run the system until a new TP is created in the slaves.
  std::unique_ptr<uni::net::ChannelTesting> initialize_keyspace(
    TestParams& p,
    std::string const& database_id,
    std::string const& table_id);

  // Runs the system for the provided number of milliseconds. Since messages usually go
  // over the network once every millisecond, we give every channel that has a message
  // a chance to deliver the message every millisecond. We also increment the clocks of
  // every server in the system (Masters and Slaves) every millisecond, adding a little
  // bit of clock skew for realism. Finally, this method takes in a message_drop_rate that
  // that's used to specify what fraction of messages are dropped. In particular,
  // 1/message_drop_rate messges are dropped.
  void run_system(TestParams& p, int32_t milliseconds, uint32_t message_drop_rate = 10);

  // Gives a chance for every Channel in nonempty_channels to deliver its
  // message with the given drop rate.
  void exchange_messages(
    std::vector<uni::net::ChannelTesting*>& nonempty_channels,
    uint32_t message_drop_rate);

  // Increments the given clock with some simulated skew.
  void increment_with_skew(uni::async::ClockTesting& clock);

  // Checks if 2 paxos logs are equal. This simply compares the maps.
  bool equals(uni::paxos::PaxosLog& paxos_log1, uni::paxos::PaxosLog& paxos_log2);

  // Simulates a node as unresponsive (due to node failure or a network partition)
  void mark_node_as_unresponsive(std::vector<std::vector<uni::net::ChannelTesting*>>& schedulers, uint32_t node);

  // Simulates a node as responsive
  void mark_node_as_responsive(std::vector<std::vector<uni::net::ChannelTesting*>>& schedulers, uint32_t node);

  // Create a client connection with the given TestingContext
  std::unique_ptr<uni::net::ChannelTesting> create_client_connection(
    uni::constants::Constants const& constants,
    std::vector<uni::net::ChannelTesting*>& server_nonempty_channels,
    std::string& server_ip,
    uni::net::Connections& server_client_connections);
};

} // integration
} // testing
} // uni


#endif // UNI_TESTING_TESTS_H
