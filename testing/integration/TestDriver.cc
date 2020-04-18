#include "TestDriver.h"

#include <boost/optional.hpp>

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

  // Create mock AsyncScheduler.
  auto slaves = std::vector<std::unique_ptr<uni::slave::TestingContext>>();
  for (auto i = 0; i < constants.num_slave_servers; i++) {
    slaves.push_back(
      std::make_unique<uni::slave::TestingContext>(
        constants,
        "universal" + std::to_string(i),
        i
      )
    );
  }

  // Holds onto Channels that aren't empty.
  auto nonempty_channels = std::vector<uni::net::ChannelTesting*>();

  // Create the IncomingMessageHandler for each universal server
  auto all_channels = std::vector<std::vector<uni::net::ChannelTesting*>>();
  for (auto i = 0; i < constants.num_slave_servers; i++) {
    all_channels.push_back(std::vector<uni::net::ChannelTesting*>());
    auto& channels = all_channels.back();
    // Populate connections with uni::net::ChannelTesting objects. We iterate over
    // the other Slaves, take their Aysnc Schedulers (which receive messages relative
    // to the current Slave), and create the uni::net::ChannelTesting object with that.
    for (auto j = 0; j < constants.num_slave_servers; j++) {
      auto& receiver_async_sheduler = slaves[j]->_scheduler;
      auto channel = std::make_unique<uni::net::ChannelTesting>(
        slaves[j]->_ip_string,
        nonempty_channels
      );
      channels.push_back(channel.get());
      slaves[i]->_connections.add_channel(std::move(channel));
    }
  }

  for (auto i = 0; i < constants.num_slave_servers; i++) {
    for (auto j = 0; j < constants.num_slave_servers; j++) {
      all_channels[i][j]->set_other_end(all_channels[j][i]);
    }
  }

  // TODO this just instatiates the tablet participants manually, since we don't have the datamaster
  // working with the tests yet. Get rid of this going forward.
  for (auto const& slave: slaves) {
    slave->_tablet_manager.handle_key_space_change({{"", "", boost::none, boost::none}});
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
