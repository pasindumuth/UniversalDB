#include <cstdlib>
#include <string>
#include <vector>

#include <async/testing/AsyncSchedulerTesting.h>
#include <constants/constants.h>
#include <net/ConnectionsOut.h>
#include <net/testing/ChannelTesting.h>
#include <paxos/PaxosLog.h>
#include <slave/IncomingMessageHandler.h>
#include <proto/client.pb.h>
#include <proto/message.pb.h>

uni::constants::Constants initialize_constants() {
  return uni::constants::Constants(5, 1610);
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
  // Initialize constants
  auto const constants = initialize_constants();

  // The ip addresses of all Universal Servers
  auto ip_strings = std::vector<std::string>();
  for (int i = 0; i < constants.num_slave_servers; i++) {
    ip_strings.push_back(std::to_string(i));
  }

  // Create mock AsyncScheduler.
  auto schedulers = std::vector<std::unique_ptr<uni::async::AsyncSchedulerTesting>>();
  for (int i = 0; i < constants.num_slave_servers; i++) {
    schedulers.push_back(std::make_unique<uni::async::AsyncSchedulerTesting>());
  }

  // Create the IncomingMessageHandler for each universal server
  auto nonempty_channels = std::vector<uni::net::ChannelTesting*>();
  auto incoming_message_handlers = std::vector<std::unique_ptr<uni::slave::IncomingMessageHandler>>();
  auto paxos_logs = std::vector<std::shared_ptr<uni::paxos::PaxosLog>>();
  for (int i = 0; i < constants.num_slave_servers; i++) {
    auto connections_out = std::make_shared<uni::net::ConnectionsOut>(constants);
    // Populate connections_out with ChannelTesting objects. We iterate over
    // the other Slaves, take their Aysnc Schedulers (which receive messages relative
    // to the current Slave), and create the ChannelTesting object with that.
    for (int j = 0; j < constants.num_slave_servers; j++) {
      auto& receiver_async_sheduler = *schedulers[j];
      auto channel = std::make_shared<uni::net::ChannelTesting>(
          constants, receiver_async_sheduler, ip_strings[i], ip_strings[j], nonempty_channels);
      connections_out->add_channel(channel);
    }

    auto paxos_log = std::make_shared<uni::paxos::PaxosLog>();
    auto paxos_instance_provider = [constants, connections_out, paxos_log](uni::paxos::index_t index) {
      return uni::paxos::SinglePaxosHandler(constants, connections_out, paxos_log, index);
    };
    auto multipaxos_handler = std::make_shared<uni::paxos::MultiPaxosHandler>(paxos_log, paxos_instance_provider);
    auto client_request_handler = std::make_shared<uni::slave::ClientRequestHandler>(multipaxos_handler);
    paxos_logs.push_back(paxos_log);
    incoming_message_handlers.push_back(std::make_unique<uni::slave::IncomingMessageHandler>(client_request_handler, multipaxos_handler));
  }

  // Connect the AsyncSchedulers to the IncomingMessageHandler for each Slave.
  for (int i = 0; i < constants.num_slave_servers; i++) {
    auto& incoming_message_handler = *incoming_message_handlers[i];
    schedulers[i]->set_callback([&incoming_message_handler](uni::net::IncomingMessage message) {
      incoming_message_handler.handle(message);
    });
  }

  // Simple test of the code.
  {
    for (int i = 0; i < 10; i++) {
      // Create a message that a client would send.
      std::string message("m" + std::to_string(i));
      auto request_message = new proto::client::ClientRequest();
      request_message->set_request_id(0);
      request_message->set_request_type(proto::client::ClientRequest_Type_READ);
      request_message->set_data(message);
      auto client_message = new proto::client::ClientMessage();
      client_message->set_allocated_request(request_message);
      auto message_wrapper = proto::message::MessageWrapper();
      message_wrapper.set_allocated_client_message(client_message);

      // Send the client message to the first Universal Slave
      auto client_endpoint_id = uni::net::endpoint_id("client", 10000);
      auto incoming_message = uni::net::IncomingMessage(client_endpoint_id, message_wrapper.SerializeAsString());
      auto& async_scheduler = *schedulers[0];
      async_scheduler.schedule_async(incoming_message);

      // Simulate the message exchanging of all Slaves until there are no more messages to send.
      std::srand(0);
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
    paxos_logs[0]->debug_print();
    paxos_logs[1]->debug_print();
    paxos_logs[2]->debug_print();
    paxos_logs[3]->debug_print();
    paxos_logs[4]->debug_print();
  }
}