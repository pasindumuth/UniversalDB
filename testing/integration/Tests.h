#ifndef UNI_TESTING_TESTS_H
#define UNI_TESTING_TESTS_H

#include <memory>
#include <vector>

#include <paxos/PaxosLog.h>
#include <proto/client.pb.h>
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

  // Makes sure that KVStore updates as expected. We must insert requests in there in a
  // controlled way.
  // TestFunction test5();

  // To prevent memory leaks, we have reflection of various memory consuming variables.
  // This includes derivations of the PaxosLog (like the request_id_set). There are methods
  // in these classes to get the size of the datastructures, so in this test, we make
  // sure they decrease together.
  // TestFunction test6();

 private:
  // Creates a MessageWrapper (the top level message that is sent over
  // the network) using data that only constitutes the client message.
  proto::message::MessageWrapper build_client_request(std::string message);

  // Goes through all paxos logs and makes sure they are compatible. That is,
  // if log index i is populated in logs l1 and l2 with values v1 and v2, then v1 = v2.
  bool verify_paxos_logs(std::vector<std::unique_ptr<uni::testing::integration::SlaveTesting>>& slaves);

  // Looks at the proposer queues and returns true iff there is a task scheduled in one.
  bool some_proposer_queue_nonempty(
    std::vector<std::unique_ptr<SlaveTesting>>& slaves);

  // While there are messages in the ProposerQueue at any node, and while are any
  // messages in the channels, keep increasing the clock on each slave. There is no
  // message dropping.
  void run_until_completion(
    std::vector<std::unique_ptr<SlaveTesting>>& slaves,
    std::vector<uni::net::ChannelTesting*>& nonempty_channels);

  // Runs for the provided number of milliseconds, each millisecond delivering n(n+1)
  // messages, with a drop rate of 0.1.
  void run_for_milliseconds(
    std::vector<std::unique_ptr<SlaveTesting>>& slaves,
    std::vector<uni::net::ChannelTesting*>& nonempty_channels,
    int milliseconds);

  // Checks if 2 paxos logs are equal. This simply compares the maps.
  bool equals(uni::paxos::PaxosLog& paxos_log1, uni::paxos::PaxosLog& paxos_log2);

  // Simulates a node as unresponsive (due to node failure or a network partition)
  void mark_node_as_unresponsive(std::vector<std::vector<uni::net::ChannelTesting*>>& schedulers, unsigned node);

  // Simulates a node as responsive
  void mark_node_as_responsive(std::vector<std::vector<uni::net::ChannelTesting*>>& schedulers, unsigned node);
};

} // integration
} // testing
} // uni


#endif // UNI_TESTING_TESTS_H
