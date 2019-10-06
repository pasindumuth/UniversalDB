#ifndef UNI_TESTING_TESTS_H
#define UNI_TESTING_TESTS_H

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
};

} // testing
} // uni


#endif // UNI_TESTING_TESTS_H
