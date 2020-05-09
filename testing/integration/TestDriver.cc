#include "TestDriver.h"

#include <boost/optional.hpp>

#include <assert/UniversalException.h>
#include <proto/message_slave.pb.h>

namespace uni {
namespace testing {
namespace integration {

uni::constants::Constants initialize_constants() {
  return uni::constants::Constants(5, 1610, 1710, 1810, 1000, 4, 1000);
}

struct SlaveObjects {
  std::vector<std::unique_ptr<uni::slave::TestingContext>> slaves;
  // This is a double array so that slave_channels[i][j] is the channel in
  // slaves[i] that connects to slaves[j].
  std::vector<std::vector<uni::net::ChannelTesting*>> slave_channels;
  // Holds onto Channels that aren't empty.
  std::vector<uni::net::ChannelTesting*> slave_nonempty_channels;
};

struct MasterObjects {
  std::vector<uni::net::EndpointId> config_endpoints;
  std::vector<std::unique_ptr<uni::master::TestingContext>> masters;
  std::vector<std::vector<uni::net::ChannelTesting*>> master_channels;
  std::vector<uni::net::ChannelTesting*> master_nonempty_channels;
};

struct MasterSlaveConnections {
  std::vector<std::vector<uni::net::ChannelTesting*>> master_slave_channels;
  std::vector<uni::net::ChannelTesting*> master_slave_nonempty_channels;
  std::vector<std::vector<uni::net::ChannelTesting*>> slave_master_channels;
  std::vector<uni::net::ChannelTesting*> slave_master_nonempty_channels;
};

void initialize_slaves(uni::constants::Constants const& constants, SlaveObjects& so) {
  // Initialize the Slave objects
  for (auto i = 0; i < constants.num_slave_servers; i++) {
    so.slaves.push_back(
      std::make_unique<uni::slave::TestingContext>(
        constants,
        "universal" + std::to_string(i),
        i
      )
    );
  }

  // Create the connections between Slaves
  for (auto i = 0; i < constants.num_slave_servers; i++) {
    so.slave_channels.push_back(std::vector<uni::net::ChannelTesting*>());
    auto& channels = so.slave_channels.back();
    for (auto j = 0; j < constants.num_slave_servers; j++) {
      auto channel = std::make_unique<uni::net::ChannelTesting>(
        so.slaves[j]->_ip_string,
        so.slave_nonempty_channels
      );
      channels.push_back(channel.get());
      so.slaves[i]->_slave_connections.add_channel(std::move(channel));
    }
  }

  // Connect the Channels of two Slaves that point to each other.
  for (auto i = 0; i < constants.num_slave_servers; i++) {
    for (auto j = 0; j < constants.num_slave_servers; j++) {
      so.slave_channels[i][j]->set_other_end(so.slave_channels[j][i]);
    }
  }
}

void initialize_masters(uni::constants::Constants const& constants, MasterObjects& mo, SlaveObjects& so) {
  // Compute config_endpoints
  for (auto i = 0; i < constants.num_master_servers; i++) {
    mo.config_endpoints.push_back({"master" + std::to_string(i), 0});
  }

  // Compute Slave Endpoints
  auto slave_endpoints = std::vector<uni::net::EndpointId>();
  for (auto const& slave : so.slaves) {
    slave_endpoints.push_back({slave->_ip_string, 0});
  }

  // Initialize the Master objects
  for (auto i = 0; i < constants.num_master_servers; i++) {
    mo.masters.push_back(
      std::make_unique<uni::master::TestingContext>(
        constants,
        mo.config_endpoints,
        slave_endpoints,
        mo.config_endpoints[i].ip_string,
        constants.num_slave_servers + i
      )
    );
  }

  // Create the connections between Masters  
  for (auto i = 0; i < constants.num_master_servers; i++) {
    mo.master_channels.push_back(std::vector<uni::net::ChannelTesting*>());
    auto& channels = mo.master_channels.back();
    for (auto j = 0; j < constants.num_master_servers; j++) {
      auto channel = std::make_unique<uni::net::ChannelTesting>(
        mo.masters[j]->_ip_string,
        mo.master_nonempty_channels
      );
      channels.push_back(channel.get());
      mo.masters[i]->_connections.add_channel(std::move(channel));
    }
  }

  // Connect the Channels of two Master that point to each other.
  for (auto i = 0; i < constants.num_master_servers; i++) {
    for (auto j = 0; j < constants.num_master_servers; j++) {
      mo.master_channels[i][j]->set_other_end(mo.master_channels[j][i]);
    }
  }
}

void connect_masters_slaves(
  uni::constants::Constants const& constants,
  SlaveObjects& so,
  MasterObjects& mo,
  MasterSlaveConnections& c
) {
  // Setup Channels in the Masters that connect to the Slaves
  for (auto i = 0; i < constants.num_master_servers; i++) {
    c.master_slave_channels.push_back(std::vector<uni::net::ChannelTesting*>());
    auto& channels = c.master_slave_channels.back();
    for (auto j = 0; j < constants.num_slave_servers; j++) {
      auto channel = std::make_unique<uni::net::ChannelTesting>(
        so.slaves[j]->_ip_string,
        c.master_slave_nonempty_channels
      );
      channels.push_back(channel.get());
      mo.masters[i]->_slave_connections.add_channel(std::move(channel));
    }
  }

  // Setup Channels in the Slaves that connect to the Masters
  for (auto i = 0; i < constants.num_slave_servers; i++) {
    c.slave_master_channels.push_back(std::vector<uni::net::ChannelTesting*>());
    auto& channels = c.slave_master_channels.back();
    for (auto j = 0; j < constants.num_master_servers; j++) {
      auto channel = std::make_unique<uni::net::ChannelTesting>(
        mo.masters[j]->_ip_string,
        c.slave_master_nonempty_channels
      );
      channels.push_back(channel.get());
      so.slaves[i]->_master_connections.add_channel(std::move(channel));
    }
  }

  // Connect the Channels of every Master-Slave and Slave-Master pair that point to each other.
  for (auto i = 0; i < constants.num_master_servers; i++) {
    for (auto j = 0; j < constants.num_slave_servers; j++) {
      c.master_slave_channels[i][j]->set_other_end(c.slave_master_channels[j][i]);
      c.slave_master_channels[j][i]->set_other_end(c.master_slave_channels[i][j]);
    }
  }
}

void TestDriver::run_test(TestFunction test) {
  // Initialize constants
  auto const constants = initialize_constants();

  auto so = SlaveObjects();
  initialize_slaves(constants, so);

  auto mo = MasterObjects();
  initialize_masters(constants, mo, so);

  auto c = MasterSlaveConnections();
  connect_masters_slaves(constants, so, mo, c);

  try {
    test({
      constants,
      so.slaves, so.slave_channels, so.slave_nonempty_channels,
      mo.masters, mo.master_channels, mo.master_nonempty_channels,
      c.master_slave_channels, c.master_slave_nonempty_channels,
      c.slave_master_channels, c.slave_master_nonempty_channels
    });
    LOG(uni::logging::Level::INFO, "[TEST PASSED]")
  } catch (uni::assert::UniversalException& e) {
    LOG(uni::logging::Level::INFO, "[TEST FAILED] " + std::string(e.what()))
  }
}

} // namespace integration
} // namespace testing
} // namespace uni
