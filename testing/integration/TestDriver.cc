#include "TestDriver.h"

#include <assert/UniversalException.h>

#include <proto/slave.pb.h>

namespace uni {
namespace testing {
namespace integration {

uni::constants::Constants initialize_constants() {
  return uni::constants::Constants(5, 1610, 1710, 1810, 1000, 4, 1000);
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
  auto slaves = std::vector<std::unique_ptr<uni::slave::TestingContext>>();
  for (auto i = 0; i < constants.num_slave_servers; i++) {
    slaves.push_back(std::make_unique<uni::slave::TestingContext>(constants));
  }

  // Holds onto Channels that aren't empty.
  auto nonempty_channels = std::vector<uni::net::ChannelTesting*>();

  // Create the IncomingMessageHandler for each universal server
  auto all_channels = std::vector<std::vector<uni::net::ChannelTesting*>>();
  for (auto i = 0; i < constants.num_slave_servers; i++) {
    auto& slave = *slaves[i];
    all_channels.push_back(std::vector<uni::net::ChannelTesting*>());
    auto& channels = all_channels.back();
    // Populate connections_out with uni::net::ChannelTesting objects. We iterate over
    // the other Slaves, take their Aysnc Schedulers (which receive messages relative
    // to the current Slave), and create the uni::net::ChannelTesting object with that.
    for (auto j = 0; j < constants.num_slave_servers; j++) {
      auto& receiver_async_sheduler = slaves[j]->scheduler;
      auto channel = std::make_shared<uni::net::ChannelTesting>(
          constants, receiver_async_sheduler, ip_strings[i], ip_strings[j], nonempty_channels);
      slave.connections_out.add_channel(channel);
      channels.push_back(channel.get());
    }
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
