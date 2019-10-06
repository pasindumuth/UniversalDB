#include <cstdlib>
#include <string>
#include <vector>

#include <assert/assert.h>
#include <async/testing/AsyncSchedulerTesting.h>
#include <net/testing/ChannelTesting.h>
#include <paxos/PaxosLog.h>
#include <proto/client.pb.h>
#include <proto/message.pb.h>
#include <testing/TestDriver.h>

// Creates a MessageWrapper (the top level message that is sent over
// the network) using data that only constitutes the client message.
proto::message::MessageWrapper build_client_request(std::string message, int request_id, proto::client::ClientRequest_Type type) {
  auto request_message = new proto::client::ClientRequest();
  request_message->set_request_id(request_id);
  request_message->set_request_type(type);
  request_message->set_data(message);
  auto client_message = new proto::client::ClientMessage();
  client_message->set_allocated_request(request_message);
  auto message_wrapper = proto::message::MessageWrapper();
  message_wrapper.set_allocated_client_message(client_message);
  return message_wrapper;
}

// More advanced test with message dropping
void test2(
    std::vector<std::unique_ptr<uni::async::AsyncSchedulerTesting>>& schedulers,
    std::vector<uni::net::ChannelTesting*>& nonempty_channels,
    std::vector<std::unique_ptr<uni::paxos::PaxosLog>>& paxos_logs) {
  for (int i = 0; i < 10; i++) {
    // Send the client message to the first Universal Slave
    auto client_endpoint_id = uni::net::endpoint_id("client", 10000);

    // Create a message that a client would send.
    auto incoming_message = uni::net::IncomingMessage(client_endpoint_id,
        build_client_request("m" + std::to_string(i), 0, proto::client::ClientRequest_Type_READ).SerializeAsString());
    schedulers[0]->schedule_async(incoming_message);

    // Simulate the message exchanging of all Slaves until there are no more messages to send.
    std::srand(i);
    while (nonempty_channels.size() > 0) {
      // We use modulus to reduce the random number to the range we want.
      // There will be minor bias with this method, but this isn't significant,
      // and so isn't a problem for us.
      int r = std::rand() % nonempty_channels.size();
      auto channel = nonempty_channels[r];
      int should_keep = std::rand() % 4;
      if (should_keep) {
        // simulate a successful delivery of the message
        channel->deliver_message();
      } else {
        // simulate a drop of the message
        channel->drop_message();
      }
    }
  }
  // Now that the simulation is done, print out the Paxos Log and see what we have.
  for (int i = 0; i < 5; i++) {
    paxos_logs[i]->debug_print();
  }
}

// Simple test of the code.
void test(
    std::vector<std::unique_ptr<uni::async::AsyncSchedulerTesting>>& schedulers,
    std::vector<uni::net::ChannelTesting*>& nonempty_channels,
    std::vector<std::unique_ptr<uni::paxos::PaxosLog>>& paxos_logs) {
  for (int i = 0; i < 10; i++) {
    // Send the client message to the first Universal Slave
    auto client_endpoint_id = uni::net::endpoint_id("client", 10000);

    // Create a message that a client would send.
    auto incoming_message = uni::net::IncomingMessage(client_endpoint_id,
        build_client_request("m" + std::to_string(i), 0, proto::client::ClientRequest_Type_READ).SerializeAsString());
    schedulers[0]->schedule_async(incoming_message);

    // Simulate the message exchanging of all Slaves until there are no more messages to send.
    std::srand(i);
    while (nonempty_channels.size() > 0) {
      // We use modulus to reduce the random number to the range we want.
      // There will be minor bias with this method, but this isn't significant,
      // and so isn't a problem for us.
      int r = std::rand() % nonempty_channels.size();
      auto channel = nonempty_channels[r];
      channel->deliver_message();
    }
  }
  // Now that the simulation is done, print out the Paxos Log and see what we have.
  for (int i = 0; i < 5; i++) {
    paxos_logs[i]->debug_print();
  }
}

/*
 * Our testing scheme is as follows. We mock out Channel and AsyncScheduler such that
 * the network becomes under the control of the Simulation thread.
 *
 * In our simulation, we have 5 Universal Slave servers. We index each server with an integer.
 * Whenever we have arrays, the position of the elements are what we use to determine the
 * Slave it's associated with.
 */
int main(int argc, char* argv[]) {
  auto test_driver = uni::testing::TestDriver();
  try {
//    test_driver.run_test(test);
    test_driver.run_test(test2);
  } catch (uni::assert::UniversalException e) {
    std::cout << e.what() << std::endl;
  }
}