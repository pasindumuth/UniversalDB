#include "TestDriver.h"

#include <async/testing/TimerAsyncSchedulerTesting.h>
#include <net/ConnectionsOut.h>
#include <paxos/MultiPaxosHandler.h>
#include <paxos/PaxosTypes.h>
#include <slave/ClientRequestHandler.h>
#include <slave/FailureDetector.h>
#include <slave/IncomingMessageHandler.h>
#include <testing/SlaveTesting.h>

namespace uni {
namespace testing {

using uni::async::ClockTesting;
using uni::async::AsyncSchedulerTesting;
using uni::async::TimerAsyncSchedulerTesting;
using uni::constants::Constants;
using uni::net::ChannelTesting;
using uni::net::ConnectionsOut;
using uni::paxos::MultiPaxosHandler;
using uni::paxos::PaxosLog;
using uni::slave::ClientRequestHandler;
using uni::slave::FailureDetector;
using uni::slave::IncomingMessageHandler;
using uni::testing::SlaveTesting;

Constants initialize_constants() {
  return Constants(5, 1610, 1710, 1810, 1000, 4);
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
  auto slaves = std::vector<std::unique_ptr<SlaveTesting>>();
  for (int i = 0; i < constants.num_slave_servers; i++) {
    auto slave = std::make_unique<SlaveTesting>();
    slave->scheduler = std::make_unique<AsyncSchedulerTesting>();
    slaves.push_back(std::move(slave));
  }

  // Holds onto Channels that aren't empty.
  auto nonempty_channels = std::vector<ChannelTesting*>();

  // Create the IncomingMessageHandler for each universal server
  auto all_channels = std::vector<std::vector<ChannelTesting*>>();
  for (int i = 0; i < constants.num_slave_servers; i++) {
    auto& slave = *slaves[i];
    slave.connections_out = std::make_unique<ConnectionsOut>(constants);
    all_channels.push_back(std::vector<ChannelTesting*>());
    auto& channels = all_channels.back();
    // Populate connections_out with ChannelTesting objects. We iterate over
    // the other Slaves, take their Aysnc Schedulers (which receive messages relative
    // to the current Slave), and create the ChannelTesting object with that.
    for (int j = 0; j < constants.num_slave_servers; j++) {
      auto& receiver_async_sheduler = *slaves[j]->scheduler;
      auto channel = std::make_shared<ChannelTesting>(
          constants, receiver_async_sheduler, ip_strings[i], ip_strings[j], nonempty_channels);
      slave.connections_out->add_channel(channel);
      channels.push_back(channel.get());
    }

    slave.clock = std::make_unique<ClockTesting>();
    slave.timer_scheduler = std::make_unique<TimerAsyncSchedulerTesting>(*slave.clock);
    slave.paxos_log = std::make_unique<PaxosLog>();
    auto paxos_instance_provider = [&slave, &constants](uni::paxos::index_t index) {
      return uni::paxos::SinglePaxosHandler(constants, *slave.connections_out, *slave.paxos_log, index);
    };
    slave.multipaxos_handler = std::make_unique<MultiPaxosHandler>(*slave.paxos_log, paxos_instance_provider);
    slave.client_request_handler = std::make_unique<ClientRequestHandler>(*slave.multipaxos_handler);
    slave.failure_detector = std::make_unique<FailureDetector>(constants, *slave.connections_out, *slave.timer_scheduler);
    slave.incoming_message_handler = std::make_unique<IncomingMessageHandler>(*slave.client_request_handler, *slave.failure_detector, *slave.multipaxos_handler);
  }

  // Connect the AsyncSchedulers to the IncomingMessageHandler for each Slave.
  for (int i = 0; i < constants.num_slave_servers; i++) {
    auto& slave = *slaves[i];
    auto& incoming_message_handler = *slave.incoming_message_handler;
    slave.scheduler->set_callback([&incoming_message_handler](uni::net::IncomingMessage message) {
      incoming_message_handler.handle(message);
    });
  }

  test(constants, slaves, all_channels, nonempty_channels);
}

} // testing
} // uni
