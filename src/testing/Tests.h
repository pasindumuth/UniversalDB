#ifndef UNI_TESTING_TESTS_H
#define UNI_TESTING_TESTS_H

#include <vector>
#include <memory>

#include <paxos/PaxosLog.h>
#include <proto/client.pb.h>
#include <proto/message.pb.h>
#include <testing/TestDriver.h>

namespace uni {
namespace testing {

/**
 * This class defines test simulations. In a test simulation, the order in which
 * Paxos messages are delivered and dropped is defined, as well as when client
 * messages are sent to the Paxos Slaves. Ideally, many different simulation
 * scenarios should be here.
 */
class Tests {
 public:
  // This is a test where a client message is sent, all Paxos messages are delivered
  // until there are no more message left in the Channels.
  TestFunction test1();

  // This is a test where a client message is sent, and a quarter of the messages
  // are dropped. The Paxos messages are dealt with until there are no more
  // messages left in the Channels.
  TestFunction test2();

 private:
  // Creates a MessageWrapper (the top level message that is sent over
  // the network) using data that only constitutes the client message.
  proto::message::MessageWrapper build_client_request(
      std::string message,
      int request_id,
      proto::client::ClientRequest_Type type);

  // Goes through all paxos logs and makes sure they are compatible. That is,
  // if log index i is populated in logs l1 and l2 with values v1 and v2, then v1 = v2.
  bool verify_paxos_logs(std::vector<std::unique_ptr<uni::paxos::PaxosLog>>& paxos_logs);
};

} // testing
} // uni


#endif // UNI_TESTING_TESTS_H
