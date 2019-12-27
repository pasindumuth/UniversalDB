#include "TestDriver.h"

#include <assert/UniversalException.h>
#include <async/testing/TimerAsyncSchedulerTesting.h>
#include <net/ConnectionsOut.h>
#include <paxos/MultiPaxosHandler.h>
#include <paxos/PaxosTypes.h>
#include <slave/ClientRequestHandler.h>
#include <slave/HeartbeatTracker.h>
#include <slave/FailureDetector.h>
#include <slave/IncomingMessageHandler.h>
#include <slave/KVStore.h>
#include <slave/LogSyncer.h>
#include <slave/ProposerQueue.h>

namespace uni {
namespace testing {
namespace integration {

using uni::async::ClockTesting;
using uni::async::AsyncSchedulerTesting;
using uni::async::TimerAsyncSchedulerTesting;
using uni::constants::Constants;
using uni::net::ChannelTesting;
using uni::net::ConnectionsOut;
using uni::paxos::MultiPaxosHandler;
using uni::paxos::PaxosLog;
using uni::slave::ClientRequestHandler;
using uni::slave::HeartbeatTracker;
using uni::slave::FailureDetector;
using uni::slave::IncomingMessageHandler;
using uni::slave::KVStore;
using uni::slave::LogSyncer;
using uni::slave::ProposerQueue;
using uni::testing::integration::SlaveTesting;

Constants initialize_constants() {
  return Constants(5, 1610, 1710, 1810, 1000, 4, 1000);
}

void TestDriver::run_test(TestFunction test) {
  // Initialize constants
  auto const constants = initialize_constants();

  // The ip addresses of all Universal Servers
  auto ip_strings = std::vector<std::string>();
  for (auto i = 0; i < constants.num_slave_servers; i++) {
    ip_strings.push_back(std::to_string(i));
  }

  // Create mock AsyncScheduler.
  auto slaves = std::vector<std::unique_ptr<SlaveTesting>>();
  for (auto i = 0; i < constants.num_slave_servers; i++) {
    auto slave = std::make_unique<SlaveTesting>();
    slave->scheduler = std::make_unique<AsyncSchedulerTesting>();
    slaves.push_back(std::move(slave));
  }

  // Holds onto Channels that aren't empty.
  auto nonempty_channels = std::vector<ChannelTesting*>();

  // Create the IncomingMessageHandler for each universal server
  auto all_channels = std::vector<std::vector<ChannelTesting*>>();
  for (auto i = 0; i < constants.num_slave_servers; i++) {
    auto& slave = *slaves[i];
    slave.connections_out = std::make_unique<ConnectionsOut>(constants);
    all_channels.push_back(std::vector<ChannelTesting*>());
    auto& channels = all_channels.back();
    // Populate connections_out with ChannelTesting objects. We iterate over
    // the other Slaves, take their Aysnc Schedulers (which receive messages relative
    // to the current Slave), and create the ChannelTesting object with that.
    for (auto j = 0; j < constants.num_slave_servers; j++) {
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
    slave.proposer_queue = std::make_unique<ProposerQueue>(*slave.timer_scheduler);
    slave.client_request_handler = std::make_unique<ClientRequestHandler>(*slave.multipaxos_handler, *slave.paxos_log, *slave.proposer_queue);
    slave.heartbeat_tracker = std::make_unique<HeartbeatTracker>();
    slave.failure_detector = std::make_unique<FailureDetector>(*slave.heartbeat_tracker, *slave.connections_out, *slave.timer_scheduler);
    slave.log_syncer = std::make_unique<LogSyncer>(constants, *slave.connections_out, *slave.timer_scheduler, *slave.paxos_log, *slave.failure_detector);
    slave.incoming_message_handler = std::make_unique<IncomingMessageHandler>(*slave.client_request_handler, *slave.heartbeat_tracker, *slave.log_syncer, *slave.multipaxos_handler);
    slave.scheduler->set_callback([&slave](uni::net::IncomingMessage message) {
      slave.incoming_message_handler->handle(message);
    });
    slave.kvstore = std::make_unique<KVStore>();
    slave.paxos_log->add_callback(slave.kvstore->get_paxos_callback());
  }

  try {
    test(constants, slaves, all_channels, nonempty_channels);
    LOG(uni::logging::Level::INFO, "[TEST PASSED]")
  } catch (uni::assert::UniversalException& e) {
    LOG(uni::logging::Level::INFO, "[TEST FAILED] " + std::string(e.what()))
  }
}

} // integration
} // testing
} // uni
