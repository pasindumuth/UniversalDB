#include "Tests.h"

#include <unordered_map>
#include <unordered_set>
#include <memory>

#include <google/protobuf/util/message_differencer.h>

#include <assert/assert.h>
#include <constants/constants.h>
#include <net/IncomingMessage.h>
#include <net/testing/ChannelTesting.h>
#include <proto/paxos.pb.h>
#include <slave/testing/TestingContext.h>
#include <utils/pbutil.h>

namespace uni {
namespace testing {
namespace integration {

TestFunction Tests::test1() {
  return [this](TestParams p) {
    auto client_dm_channel = initialize_keyspace(p, "", "");

    // Send the client message to the first Universal Slave
    auto client_channels = std::vector<std::unique_ptr<uni::net::ChannelTesting>>();
    for (auto i = 0; i < p.slaves.size(); i++) {
      client_channels.push_back(create_client_connection(
        p.constants, p.slave_nonempty_channels, p.slaves[i]->_ip_string, p.slaves[i]->_client_connections));
    }
    for (auto i = 0; i < 300; i++) {
      // Send a client message to some node in the Paxos Group. The node is
      // chosen randomly.
      client_channels[std::rand() % client_channels.size()]->queue_send(
        build_client_request("m" + std::to_string(i)).SerializeAsString());
      // Run the nodes and network for 1ms.
      run_system(p, 1);
    }
    // Now that the simulation is done, print out the Paxos Log and see what we have.
    print_paxos_logs(p.slaves);

    UNIVERSAL_ASSERT_MESSAGE(
      verify_all_paxos_logs(p.slaves),
      "The Paxos Logs should agree with one another.")
  };
}

TestFunction Tests::test2() {
  return [this](TestParams p) {
    auto client_dm_channel = initialize_keyspace(p, "", "");

    auto nodes_failed = 0;
    // Send the client message to the first Universal Slave
    auto client_channels = std::vector<std::unique_ptr<uni::net::ChannelTesting>>();
    for (auto i = 0; i < p.slaves.size(); i++) {
      client_channels.push_back(create_client_connection(
        p.constants, p.slave_nonempty_channels, p.slaves[i]->_ip_string, p.slaves[i]->_client_connections));
    }
    for (auto i = 0; i < 300; i++) {
      // Send a client message to some node in the Paxos Group. The node is
      // chosen randomly.
      client_channels[std::rand() % client_channels.size()]->queue_send(
        build_client_request("m" + std::to_string(i)).SerializeAsString());
      // Run the nodes and network for 1ms.
      run_system(p, 1);
      // Fail a node if there isn't already a failed node.
      if (nodes_failed < 3 && (std::rand() % 40 == 0)) {
        if (nodes_failed == 0) {
          mark_node_as_unresponsive(p.slave_channels, 2);
        } else if (nodes_failed == 1) {
          mark_node_as_unresponsive(p.slave_channels, 3);
        } else if (nodes_failed == 2) {
          mark_node_as_unresponsive(p.slave_channels, 4);
        }
        nodes_failed++;
      }
    }
    // Now that the simulation is done, print out the Paxos Log and see what we have.
    print_paxos_logs(p.slaves);

    UNIVERSAL_ASSERT_MESSAGE(
      verify_all_paxos_logs(p.slaves),
      "The Paxos Logs should agree with one another.")
  };
}

TestFunction Tests::test3() {
  return [this](TestParams p) {
    auto client_dm_channel = initialize_keyspace(p, "", "");

    bool passed = true;
    // Wait one heartbeat cycle so that the nodes can send each other a heartbeat
    for (auto i = 0; i < p.constants.heartbeat_period; i++) {
      for (auto j = 0; j < p.slaves.size(); j++) {
        p.slaves[j]->_clock.increment_time(1);
      }
      // Exchange all the messages that need to be exchanged
      while (p.slave_nonempty_channels.size() > 0) {
        p.slave_nonempty_channels[0]->deliver_message();
      }
    }
    // All failure detectors should report that all of the p.slaves are still alive.
    for (auto const& slave : p.slaves) {
      UNIVERSAL_ASSERT_MESSAGE(
        slave->_heartbeat_tracker.alive_endpoints().size() == p.slaves.size(),
        "The Paxos Logs should agree with one another.")
    }
    // Now kill one of the slaves
    mark_node_as_unresponsive(p.slave_channels, 0);
    // Increment the clock on all other slaves an amount such that
    // they will detect the failure
    for (auto i = 0; i < p.constants.heartbeat_failure_threshold * p.constants.heartbeat_period; i++) {
      for (auto j = 1; j < p.slaves.size(); j++) {
        p.slaves[j]->_clock.increment_time(1);
      }
      // Exchange all the messages that need to be exchanged
      while (p.slave_nonempty_channels.size() > 0) {
        p.slave_nonempty_channels[0]->deliver_message();
      }
    }
    for (auto i = 1; i < p.slaves.size(); i++) {
      UNIVERSAL_ASSERT_MESSAGE(
        p.slaves[i]->_heartbeat_tracker.alive_endpoints().size() == p.slaves.size() - 1,
        "All failure detectors should report that one slave is dead.")
    }
  };
}

// TODO this test is so bad that we don't even need the LogSyncer running for it to pass.
TestFunction Tests::test4() {
  return [this](TestParams p) {
    auto client_dm_channel = initialize_keyspace(p, "", "");

    auto client_channels = std::vector<std::unique_ptr<uni::net::ChannelTesting>>();
    for (auto i = 0; i < p.slaves.size(); i++) {
      client_channels.push_back(create_client_connection(
        p.constants, p.slave_nonempty_channels, p.slaves[i]->_ip_string, p.slaves[i]->_client_connections));
    }
    auto client_request_id = 0;
    auto simulate_client_requests = [&](int32_t request_count, int32_t target_slave) {
      // Send the client message to the first Universal Slave
      auto final_request_id = client_request_id + request_count;
      for (; client_request_id < final_request_id; client_request_id++) {
        // Send a client message to some node in the Paxos Group. The node is
        // chosen randomly.
        client_channels[target_slave]->queue_send(
          build_client_request("m" + std::to_string(client_request_id)).SerializeAsString());
        run_system(p, 5);
      }
    };
    mark_node_as_unresponsive(p.slave_channels, 0);

    simulate_client_requests(10, 1);
    mark_node_as_responsive(p.slave_channels, 0);
    mark_node_as_unresponsive(p.slave_channels, 1);

    simulate_client_requests(10, 2);
    mark_node_as_responsive(p.slave_channels, 1);
    mark_node_as_unresponsive(p.slave_channels, 2);

    simulate_client_requests(10, 3);
    mark_node_as_responsive(p.slave_channels, 2);
    mark_node_as_unresponsive(p.slave_channels, 3);

    simulate_client_requests(10, 4);
    mark_node_as_responsive(p.slave_channels, 3);
    mark_node_as_unresponsive(p.slave_channels, 4);

    simulate_client_requests(10, 0);
    mark_node_as_responsive(p.slave_channels, 4);
    mark_node_as_unresponsive(p.slave_channels, 0);

    mark_node_as_responsive(p.slave_channels, 0);

    UNIVERSAL_ASSERT_MESSAGE(verify_all_paxos_logs(p.slaves), "The Paxos Logs should agree with one another.")

    // The following counts all corresponding pairs of paxos logs across
    // the slaves that are compatible with each other.
    auto initial_equal_logs = 0; // The total number of logs that are the same (across different TPs and slave paxos logs)
    auto possible_equals_logs = 0; // The maximum possible value for the above variable.
    auto const initial_aligned_logs = get_aligned_logs(p.slaves);
    for (auto i = 0; i < initial_aligned_logs.size(); i++) {
      auto const& logs = get_present_logs(initial_aligned_logs[i]);
      for (auto j = 0; j < logs.size(); j++) {
        for (auto k = j + 1; k < logs.size(); k++) {
          if (equals(*logs[j], *logs[k])) {
            initial_equal_logs += 1;
          }
          possible_equals_logs += 1;
        }
      }
    }

    UNIVERSAL_ASSERT_MESSAGE(initial_equal_logs < possible_equals_logs,
          "Not all pairs of PaxosLogs should be equal.")

    // Make sure that at least one heartbeat is sent to ensure a leader is known
    // (plus an extra 5 milliseconds to make sure all messages are sent).
    run_system(p, p.constants.heartbeat_period + 5);
    // Make sure one sync message is sent to ensure at least one syncing
    // (plus an extra 5 milliseconds to make sure all messages are sent).
    run_system(p, p.constants.log_syncer_period + 5);

    UNIVERSAL_ASSERT_MESSAGE(verify_all_paxos_logs(p.slaves), "The Paxos Logs should agree with one another.")
    // All of the PaxosLogs should now be equal
    auto final_equal_logs = 0;
    auto const final_aligned_logs = get_aligned_logs(p.slaves);
    for (auto i = 0; i < final_aligned_logs.size(); i++) {
      auto const& logs = get_present_logs(final_aligned_logs[i]);
      for (auto j = 0; j < logs.size(); j++) {
        for (auto k = j + 1; k < logs.size(); k++) {
          if (equals(*logs[j], *logs[k])) {
            final_equal_logs += 1;
          }
        }
      }
    }

    UNIVERSAL_ASSERT_MESSAGE(initial_equal_logs < final_equal_logs,
          "Paxos Logs should all be equal.")
    for (auto const& [tablet_id, tp] : p.slaves[0]->_tablet_manager.get_tps()) {
      LOG(uni::logging::Level::DEBUG, tp->_kvstore.debug_string());
    }
  };
}

// TODO(pasindu): Get rid of these in due time.
static int64_t request_id = 0;
static int64_t timestamp = 0;

proto::message::MessageWrapper Tests::build_client_request(std::string message) {
  auto message_wrapper = proto::message::MessageWrapper();
  auto client_message = new proto::message::client::ClientMessage();
  auto request_message = new proto::message::client::ClientRequest();
  request_message->set_request_id(std::to_string(++request_id));
  request_message->set_request_type(proto::message::client::ClientRequest::WRITE);
  request_message->set_key(std::string("key"));
  request_message->set_allocated_value(uni::utils::pb::string(message));
  request_message->set_allocated_timestamp(uni::utils::pb::uint64(++timestamp));
  client_message->set_allocated_request(request_message);
  message_wrapper.set_allocated_client_message(client_message);
  return message_wrapper;
}

// Looks at the proposer queues and returns true iff there is a task scheduled in one.
bool Tests::some_async_queue_nonempty(
  std::vector<std::unique_ptr<uni::slave::TestingContext>>& slaves) {
    for (auto const& slave: slaves) {
      if (!slave->_async_queue.empty()) {
        return true;
      }
    }
    return false;
}

std::unique_ptr<uni::net::ChannelTesting> Tests::initialize_keyspace(
  TestParams& p,
  std::string const& database_id,
  std::string const& table_id
) {
  auto client_dm_channel = create_client_connection(
    p.constants, p.master_nonempty_channels, p.masters[0]->_ip_string, p.masters[0]->_client_connections);

  auto message_wrapper = proto::message::MessageWrapper();
  auto client_message = new proto::message::client::ClientMessage();
  auto find_key_request = new proto::message::client::FindKeyRangeRequest();
  find_key_request->set_database_id(database_id);
  find_key_request->set_table_id(table_id);
  find_key_request->set_key("key");
  client_message->set_allocated_find_key_range_request(find_key_request);
  message_wrapper.set_allocated_client_message(client_message);
  client_dm_channel->queue_send(message_wrapper.SerializeAsString());

  run_system(p, 200, 1000);

  return client_dm_channel;
}

void Tests::run_system(TestParams& p, int32_t milliseconds, uint32_t message_drop_rate) {
  for (auto t = 0; t < milliseconds; t++) {
    for (auto const& slave: p.slaves) {
      increment_with_skew(slave->_clock);
    }
    for (auto const& master: p.masters) {
      increment_with_skew(master->_clock);
    }
    exchange_messages(p.slave_nonempty_channels, message_drop_rate);
    exchange_messages(p.master_nonempty_channels, message_drop_rate);
    exchange_messages(p.master_slave_nonempty_channels, message_drop_rate);
    exchange_messages(p.slave_master_nonempty_channels, message_drop_rate);
  }
}

void Tests::exchange_messages(
  std::vector<uni::net::ChannelTesting*>& nonempty_channels,
  uint32_t message_drop_rate
) {
  auto n = nonempty_channels.size();
  auto channels_sent = std::unordered_set<uni::net::ChannelTesting*>();
  for (auto i = 0; i < n; i++) {
    if (std::rand() % 10 < 9) {
      // This if statement helps prevent the exact same number of
      // of messages being exchanged everytime.
      auto channels_not_sent = std::vector<uni::net::ChannelTesting*>();
      for (auto const& channel : nonempty_channels) {
        if (channels_sent.find(channel) == channels_sent.end()) {
          channels_not_sent.push_back(channel);
        }
      }
      if (channels_not_sent.size() > 0) {
        auto& channel = channels_not_sent[std::rand() % channels_not_sent.size()];
        if (std::rand() % message_drop_rate < (message_drop_rate - 1)) {
          // simulate a successful delivery of the message
          channel->deliver_message();
        } else {
          // simulate a drop of the message
          channel->drop_message();
        }
        channels_sent.insert(channel);
      } else {
        break;
      }
    }
  }
}

void Tests::increment_with_skew(uni::async::ClockTesting& clock) {
  if (std::rand() % 100 < 99) {
    // This is a fairly naive method of producing skew; it doesn't
    // simulate clocks that have slightly different speeds.
    clock.increment_time(1); 
  }
}

std::vector<std::vector<boost::optional<uni::paxos::PaxosLog*>>> Tests::get_aligned_logs(
  std::vector<std::unique_ptr<uni::slave::TestingContext>>& slaves
) {
  auto aligned_logs = std::vector<std::vector<boost::optional<uni::paxos::PaxosLog*>>>();
  auto slave_paxos_logs = std::vector<boost::optional<uni::paxos::PaxosLog*>>();
  auto tablet_ids = std::vector<uni::slave::TabletId>();
  for (auto const& slave : slaves) {
    slave_paxos_logs.push_back(&slave->_paxos_log);
    for (auto const& [tablet_id, tp] : slave->_tablet_manager.get_tps()) {
      tablet_ids.push_back(tablet_id);
    }
  }
  aligned_logs.push_back(slave_paxos_logs);
  for (auto const& tablet_id : tablet_ids) {
    auto paxos_logs = std::vector<boost::optional<uni::paxos::PaxosLog*>>();
    for (auto const& slave : slaves) {
      auto const& tp_map = slave->_tablet_manager.get_tps();
      auto const& it = tp_map.find(tablet_id);
      if (it != tp_map.end()) {
        paxos_logs.push_back(&it->second->_paxos_log);
      } else {
        paxos_logs.push_back(boost::none);
      }
    }
    aligned_logs.push_back(paxos_logs);
  }
  return aligned_logs;
}

std::vector<uni::paxos::PaxosLog*> Tests::get_present_logs(
  std::vector<boost::optional<uni::paxos::PaxosLog*>> logs
) {
  auto present_logs = std::vector<uni::paxos::PaxosLog*>();
  for (auto const& log : logs) {
    if (log) {
      present_logs.push_back(log.get());
    }
  }
  return present_logs;
}

bool Tests::verify_all_paxos_logs(std::vector<std::unique_ptr<uni::slave::TestingContext>>& slaves) {
  // To verify the logs, we iterate through each one, adding each entry into a
  // Global Paxos Log. If there is an inconsistency in this process, this means
  // that the Paxos Logs aren't consistent. Otherwise, they are consistent.
  auto aligned_logs = get_aligned_logs(slaves);
  for (auto const& logs : aligned_logs) {
    if (!verify_paxos_logs(get_present_logs(logs))) {
      return false;
    }
  }
  return true;
}

bool Tests::verify_paxos_logs(std::vector<uni::paxos::PaxosLog*> paxos_logs) {
  // To verify the logs, we iterate through each one, adding each entry into a
  // Global Paxos Log. If there is an inconsistency in this process, this means
  // that the Paxos Logs aren't consistent. Otherwise, they are consistent.
  std::unordered_map<uni::paxos::index_t, proto::paxos::PaxosLogEntry const> global_log;
  for (auto const& paxos_log : paxos_logs) {
    for (auto const& [index, entry] : paxos_log->get_log()) {
      auto it = global_log.find(index);
      if (it == global_log.end()) {
        global_log.insert({index, entry});
      } else {
        if (it->second.SerializeAsString() != entry.SerializeAsString()) {
          return false;
        }
      }
    }
  }
  return true;
}

void Tests::print_paxos_logs(std::vector<std::unique_ptr<uni::slave::TestingContext>>& slaves) {
  for (auto const& slave: slaves) {
    LOG(uni::logging::Level::DEBUG, slave->_paxos_log.debug_string())
    for (auto const& [tablet_id, tp] : slave->_tablet_manager.get_tps()) {
      LOG(uni::logging::Level::DEBUG, tp->_paxos_log.debug_string())
    }
  }
}

bool Tests::equals(uni::paxos::PaxosLog& paxos_log1, uni::paxos::PaxosLog& paxos_log2) {
  auto log1 = paxos_log1.get_log();
  auto log2 = paxos_log2.get_log();
  if (log1.size() != log2.size()) {
    return false;
  }
  for (auto const& [index, entry] : log1) {
    auto it = log2.find(index);
    if (it == log2.end() || !google::protobuf::util::MessageDifferencer::Equivalent(it->second, entry)) {
      return false;
    }
  }
  return true;
}

void Tests::mark_node_as_unresponsive(std::vector<std::vector<uni::net::ChannelTesting*>>& channels, uint32_t node) {
  for (auto i = 0; i < channels.size(); i++) {
    channels[node][i]->set_connection_state(false);
    channels[i][node]->set_connection_state(false);
  }
}

void Tests::mark_node_as_responsive(std::vector<std::vector<uni::net::ChannelTesting*>>& channels, uint32_t node) {
  for (auto i = 0; i < channels.size(); i++) {
    channels[node][i]->set_connection_state(true);
    channels[i][node]->set_connection_state(true);
  }
}

std::unique_ptr<uni::net::ChannelTesting> Tests::create_client_connection(
  uni::constants::Constants const& constants,
  std::vector<uni::net::ChannelTesting*>& server_nonempty_channels,
  std::string& server_ip,
  uni::net::Connections& server_client_connections
) {
  auto channel = std::make_unique<uni::net::ChannelTesting>(
    "client",
    server_nonempty_channels
  );
  auto client_channel = std::make_unique<uni::net::ChannelTesting>(
    server_ip,
    server_nonempty_channels
  );
  channel->set_other_end(client_channel.get());
  client_channel->set_other_end(channel.get());
  server_client_connections.add_channel(std::move(channel));
  return client_channel;
}

} // integration
} // testing
} // uni
