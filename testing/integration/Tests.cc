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
  return [this](
      uni::constants::Constants const& constants,
      std::vector<std::unique_ptr<uni::slave::TestingContext>>& slaves,
      std::vector<std::vector<uni::net::ChannelTesting*>>& all_channels,
      std::vector<uni::net::ChannelTesting*>& nonempty_channels) {
    // Send the client message to the first Universal Slave
    auto client_channels = std::vector<std::unique_ptr<uni::net::ChannelTesting>>();
    for (auto i = 0; i < slaves.size(); i++) {
      client_channels.push_back(create_client_connection(
        constants, nonempty_channels, *slaves[i]));
    }
    for (auto i = 0; i < 300; i++) {
      // Send a client message to some node in the Paxos Group. The node is
      // chosen randomly.
      client_channels[std::rand() % client_channels.size()]->queue_send(
        build_client_request("m" + std::to_string(i)).SerializeAsString());
      // Run the nodes and network for 1ms.
      run_for_milliseconds(slaves, nonempty_channels, 1);
    }
    // Now that the simulation is done, print out the Paxos Log and see what we have.
    print_paxos_logs(slaves);

    UNIVERSAL_ASSERT_MESSAGE(
      verify_all_paxos_logs(slaves),
      "The Paxos Logs should agree with one another.")
  };
}

TestFunction Tests::test2() {
  return [this](
      uni::constants::Constants const& constants,
      std::vector<std::unique_ptr<uni::slave::TestingContext>>& slaves,
      std::vector<std::vector<uni::net::ChannelTesting*>>& all_channels,
      std::vector<uni::net::ChannelTesting*>& nonempty_channels) {
    auto nodes_failed = 0;
    // Send the client message to the first Universal Slave
    auto client_channels = std::vector<std::unique_ptr<uni::net::ChannelTesting>>();
    for (auto i = 0; i < slaves.size(); i++) {
      client_channels.push_back(create_client_connection(
        constants, nonempty_channels, *slaves[i]));
    }
    for (auto i = 0; i < 300; i++) {
      // Send a client message to some node in the Paxos Group. The node is
      // chosen randomly.
      client_channels[std::rand() % client_channels.size()]->queue_send(
        build_client_request("m" + std::to_string(i)).SerializeAsString());
      // Run the nodes and network for 1ms.
      run_for_milliseconds(slaves, nonempty_channels, 1);
      // Fail a node if there isn't already a failed node.
      if (nodes_failed < 3 && (std::rand() % 40 == 0)) {
        if (nodes_failed == 0) {
          mark_node_as_unresponsive(all_channels, 2);
        } else if (nodes_failed == 1) {
          mark_node_as_unresponsive(all_channels, 3);
        } else if (nodes_failed == 2) {
          mark_node_as_unresponsive(all_channels, 4);
        }
        nodes_failed++;
      }
    }
    // Now that the simulation is done, print out the Paxos Log and see what we have.
    print_paxos_logs(slaves);

    UNIVERSAL_ASSERT_MESSAGE(
      verify_all_paxos_logs(slaves),
      "The Paxos Logs should agree with one another.")
  };
}

TestFunction Tests::test3() {
  return [this](
      uni::constants::Constants const& constants,
      std::vector<std::unique_ptr<uni::slave::TestingContext>>& slaves,
      std::vector<std::vector<uni::net::ChannelTesting*>>& all_channels,
      std::vector<uni::net::ChannelTesting*>& nonempty_channels) {
    bool passed = true;
    // Wait one heartbeat cycle so that the nodes can send each other a heartbeat
    for (auto i = 0; i < constants.heartbeat_period; i++) {
      for (auto j = 0; j < slaves.size(); j++) {
        slaves[j]->clock.increment_time(1);
      }
      // Exchange all the messages that need to be exchanged
      while (nonempty_channels.size() > 0) {
        nonempty_channels[0]->deliver_message();
      }
    }
    // All failure detectors should report that all of the slaves are still alive.
    for (auto const& slave : slaves) {
      UNIVERSAL_ASSERT_MESSAGE(
        slave->heartbeat_tracker.alive_endpoints().size() == slaves.size(),
        "The Paxos Logs should agree with one another.")
    }
    // Now kill one of the slaves
    mark_node_as_unresponsive(all_channels, 0);
    // Increment the clock on all other slaves an amount such that
    // they will detect the failure
    for (auto i = 0; i < constants.heartbeat_failure_threshold * constants.heartbeat_period; i++) {
      for (auto j = 1; j < slaves.size(); j++) {
        slaves[j]->clock.increment_time(1);
      }
      // Exchange all the messages that need to be exchanged
      while (nonempty_channels.size() > 0) {
        nonempty_channels[0]->deliver_message();
      }
    }
    for (auto i = 1; i < slaves.size(); i++) {
      UNIVERSAL_ASSERT_MESSAGE(
        slaves[i]->heartbeat_tracker.alive_endpoints().size() == slaves.size() - 1,
        "All failure detectors should report that one slave is dead.")
    }
  };
}

// TODO this test is so bad that we don't even need the LogSyncer running for it to pass.
TestFunction Tests::test4() {
  return [this](
      uni::constants::Constants const& constants,
      std::vector<std::unique_ptr<uni::slave::TestingContext>>& slaves,
      std::vector<std::vector<uni::net::ChannelTesting*>>& all_channels,
      std::vector<uni::net::ChannelTesting*>& nonempty_channels) {
    auto client_channels = std::vector<std::unique_ptr<uni::net::ChannelTesting>>();
    for (auto i = 0; i < slaves.size(); i++) {
      client_channels.push_back(create_client_connection(
        constants, nonempty_channels, *slaves[i]));
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
        run_for_milliseconds(slaves, nonempty_channels, 5);
      }
    };
    mark_node_as_unresponsive(all_channels, 0);

    simulate_client_requests(10, 1);
    mark_node_as_responsive(all_channels, 0);
    mark_node_as_unresponsive(all_channels, 1);

    simulate_client_requests(10, 2);
    mark_node_as_responsive(all_channels, 1);
    mark_node_as_unresponsive(all_channels, 2);

    simulate_client_requests(10, 3);
    mark_node_as_responsive(all_channels, 2);
    mark_node_as_unresponsive(all_channels, 3);

    simulate_client_requests(10, 4);
    mark_node_as_responsive(all_channels, 3);
    mark_node_as_unresponsive(all_channels, 4);

    simulate_client_requests(10, 0);
    mark_node_as_responsive(all_channels, 4);
    mark_node_as_unresponsive(all_channels, 0);

    mark_node_as_responsive(all_channels, 0);

    UNIVERSAL_ASSERT_MESSAGE(verify_all_paxos_logs(slaves), "The Paxos Logs should agree with one another.")

    // The following counts all corresponding pairs of paxos logs across
    // the slaves that are compatible with each other.
    auto initial_equal_logs = 0; // The total number of logs that are the same (across different TPs and slave paxos logs)
    auto possible_equals_logs = 0; // The maximum possible value for the above variable.
    auto const initial_aligned_logs = get_aligned_logs(slaves);
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
    run_for_milliseconds(slaves, nonempty_channels, constants.heartbeat_period + 5);
    // Make sure one sync message is sent to ensure at least one syncing
    // (plus an extra 5 milliseconds to make sure all messages are sent).
    run_for_milliseconds(slaves, nonempty_channels, constants.log_syncer_period + 5);

    UNIVERSAL_ASSERT_MESSAGE(verify_all_paxos_logs(slaves), "The Paxos Logs should agree with one another.")
    // All of the PaxosLogs should now be equal
    auto final_equal_logs = 0;
    auto const final_aligned_logs = get_aligned_logs(slaves);
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
    for (auto const& [tablet_id, tp] : slaves[0]->slave_handler.get_tps()) {
      LOG(uni::logging::Level::DEBUG, tp->kvstore.debug_string());
    }
  };
}

// TODO(pasindu): Get rid of these in due time.
static int64_t request_id = 0;
static int64_t timestamp = 0;

proto::message::MessageWrapper Tests::build_client_request(std::string message) {
  auto message_wrapper = proto::message::MessageWrapper();
  auto client_message = new proto::client::ClientMessage();
  auto request_message = new proto::client::ClientRequest();
  request_message->set_request_id(std::to_string(++request_id));
  request_message->set_request_type(proto::client::ClientRequest::WRITE);
  request_message->set_allocated_key(uni::utils::pb::string("key"));
  request_message->set_allocated_value(uni::utils::pb::string(message));
  request_message->set_allocated_timestamp(uni::utils::pb::uint64(++timestamp));
  client_message->set_allocated_request(request_message);
  message_wrapper.set_allocated_client_message(client_message);
  return message_wrapper;
}

// Looks at the proposer queues and returns true iff there is a task scheduled in one.
bool Tests::some_proposer_queue_nonempty(
  std::vector<std::unique_ptr<uni::slave::TestingContext>>& slaves) {
    for (auto const& slave: slaves) {
      if (!slave->proposer_queue.empty()) {
        return true;
      }
    }
    return false;
}

void Tests::run_until_completion(
  std::vector<std::unique_ptr<uni::slave::TestingContext>>& slaves,
  std::vector<uni::net::ChannelTesting*>& nonempty_channels) {
    while (some_proposer_queue_nonempty(slaves) || nonempty_channels.size() > 0) {
      run_for_milliseconds(slaves, nonempty_channels, 1);
    }
}

void Tests::run_for_milliseconds(
  std::vector<std::unique_ptr<uni::slave::TestingContext>>& slaves,
  std::vector<uni::net::ChannelTesting*>& nonempty_channels,
  int32_t milliseconds) {
    // Since we increase the clocks by 1ms, we assume one message is passed along a
    // channel on average (remember there 2 channels between 2 nodes, one for each direction).
    auto n = slaves.size() * (slaves.size() + 1); // the number of messages to exchange
    for (auto t = 0; t < milliseconds; t++) {
      for (auto const& slave: slaves) {
        if (std::rand() % 100 < 99) {
          // This if statement helps simulate unsynchronized clocks. This is a fairly
          // naive method; it doesn't simulate clocks that have slightly different speeds.
          slave->clock.increment_time(1); 
        }
      }
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
            if (std::rand() % 10 < 9) {
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
}

std::vector<std::vector<boost::optional<uni::paxos::PaxosLog*>>> Tests::get_aligned_logs(
  std::vector<std::unique_ptr<uni::slave::TestingContext>>& slaves
) {
  auto aligned_logs = std::vector<std::vector<boost::optional<uni::paxos::PaxosLog*>>>();
  auto slave_paxos_logs = std::vector<boost::optional<uni::paxos::PaxosLog*>>();
  auto tablet_ids = std::vector<uni::slave::TabletId>();
  for (auto const& slave : slaves) {
    slave_paxos_logs.push_back(&slave->paxos_log);
    for (auto const& [tablet_id, tp] : slave->slave_handler.get_tps()) {
      tablet_ids.push_back(tablet_id);
    }
  }
  aligned_logs.push_back(slave_paxos_logs);
  for (auto const& tablet_id : tablet_ids) {
    auto paxos_logs = std::vector<boost::optional<uni::paxos::PaxosLog*>>();
    for (auto const& slave : slaves) {
      auto const& tp_map = slave->slave_handler.get_tps();
      auto const& it = tp_map.find(tablet_id);
      if (it != tp_map.end()) {
        paxos_logs.push_back(&it->second->paxos_log);
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
    LOG(uni::logging::Level::DEBUG, slave->paxos_log.debug_string())
    for (auto const& [tablet_id, tp] : slave->slave_handler.get_tps()) {
      LOG(uni::logging::Level::DEBUG, tp->paxos_log.debug_string())
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
  std::vector<uni::net::ChannelTesting*>& nonempty_channels,
  uni::slave::TestingContext& slave
) {
  auto channel = std::make_unique<uni::net::ChannelTesting>(
    "client",
    nonempty_channels
  );
  auto client_channel = std::make_unique<uni::net::ChannelTesting>(
    slave.ip_string,
    nonempty_channels
  );
  channel->set_other_end(client_channel.get());
  client_channel->set_other_end(channel.get());
  slave.client_connections_in.add_channel(std::move(channel));
  return client_channel;
}

} // integration
} // testing
} // uni
