#include "TestDriver.h"

#include <constants/constants.h>
#include <net/ConnectionsOut.h>
#include <paxos/MultiPaxosHandler.h>
#include <paxos/PaxosTypes.h>
#include <slave/ClientRequestHandler.h>
#include <slave/IncomingMessageHandler.h>

namespace uni {
namespace testing {

using uni::async::AsyncSchedulerTesting;
using uni::constants::Constants;
using uni::net::ChannelTesting;
using uni::net::ConnectionsOut;
using uni::paxos::MultiPaxosHandler;
using uni::paxos::PaxosLog;
using uni::slave::ClientRequestHandler;
using uni::slave::IncomingMessageHandler;

Constants initialize_constants() {
  return Constants(5, 1610, 1710);
}

void TestDriver::run_test(TestFunction test) {
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

  // Holds onto Channels that aren't empty.
  auto nonempty_channels = std::vector<ChannelTesting*>();

  // Create the IncomingMessageHandler for each universal server
  auto connections_outs = std::vector<std::unique_ptr<ConnectionsOut>>();
  auto multipaxos_handlers = std::vector<std::unique_ptr<MultiPaxosHandler>>();
  auto client_request_handlers = std::vector<std::unique_ptr<ClientRequestHandler>>();
  auto incoming_message_handlers = std::vector<std::unique_ptr<IncomingMessageHandler>>();
  auto paxos_logs = std::vector<std::unique_ptr<PaxosLog>>();
  for (int i = 0; i < constants.num_slave_servers; i++) {
    connections_outs.push_back(std::make_unique<ConnectionsOut>(constants));
    auto& connections_out = *connections_outs.back();
    // Populate connections_out with ChannelTesting objects. We iterate over
    // the other Slaves, take their Aysnc Schedulers (which receive messages relative
    // to the current Slave), and create the ChannelTesting object with that.
    for (int j = 0; j < constants.num_slave_servers; j++) {
      auto& receiver_async_sheduler = *schedulers[j];
      auto channel = std::make_shared<ChannelTesting>(
          constants, receiver_async_sheduler, ip_strings[i], ip_strings[j], nonempty_channels);
      connections_out.add_channel(channel);
    }

    paxos_logs.push_back(std::make_unique<PaxosLog>());
    auto& paxos_log = *paxos_logs.back();
    auto paxos_instance_provider = [&constants, &connections_out, &paxos_log](uni::paxos::index_t index) {
      return uni::paxos::SinglePaxosHandler(constants, connections_out, paxos_log, index);
    };
    multipaxos_handlers.push_back(std::make_unique<MultiPaxosHandler>(paxos_log, paxos_instance_provider));
    auto& multipaxos_handler = *multipaxos_handlers.back();
    client_request_handlers.push_back(std::make_unique<ClientRequestHandler>(multipaxos_handler));
    auto& client_request_handler = *client_request_handlers.back();
    incoming_message_handlers.push_back(std::make_unique<IncomingMessageHandler>(client_request_handler, multipaxos_handler));
    auto& incoming_message_handler = *incoming_message_handlers.back();
  }

  // Connect the AsyncSchedulers to the IncomingMessageHandler for each Slave.
  for (int i = 0; i < constants.num_slave_servers; i++) {
    auto& incoming_message_handler = *incoming_message_handlers[i];
    schedulers[i]->set_callback([&incoming_message_handler](uni::net::IncomingMessage message) {
      incoming_message_handler.handle(message);
    });
  }

  test(schedulers, nonempty_channels, paxos_logs);
}


} // testing
} // uni

