#include "Tests.h"

#include <unordered_map>
#include <unordered_set>
#include <memory>

#include <google/protobuf/util/message_differencer.h>

#include <assert/assert.h>
#include <async/testing/AsyncSchedulerTesting.h>
#include <constants/constants.h>
#include <logging/log.h>
#include <net/IncomingMessage.h>
#include <net/testing/ChannelTesting.h>
#include <proto/paxos.pb.h>
#include <testing/SlaveTesting.h>

namespace uni {
namespace testing {

using google::protobuf::util::MessageDifferencer;
using proto::message::MessageWrapper;
using uni::constants::Constants;
using uni::async::AsyncSchedulerTesting;
using uni::net::ChannelTesting;
using uni::paxos::PaxosLog;
using uni::net::IncomingMessage;
using uni::testing::SlaveTesting;

TestFunction Tests::test1() {
  return [this](
      Constants const& constants,
      std::vector<std::unique_ptr<SlaveTesting>>& slaves,
      std::vector<std::vector<ChannelTesting*>>& all_channels,
      std::vector<ChannelTesting*>& nonempty_channels) {
    // Send the client message to the first Universal Slave
    auto client_endpoint_id = uni::net::endpoint_id("client", 10000);
    for (int i = 0; i < 300; i++) {
      // Send a client message to some node in the Paxos Group. The node is
      // chosen randomly.
      auto incoming_message = IncomingMessage(client_endpoint_id,
          build_client_request("m" + std::to_string(i)).SerializeAsString());
      slaves[std::rand() % slaves.size()]->scheduler->queue_message(incoming_message);
      // Run the nodes and network for 1ms.
      run_for_milliseconds(slaves, nonempty_channels, 1);
    }
    // Now that the simulation is done, print out the Paxos Log and see what we have.
    for (int i = 0; i < 5; i++) {
      LOG(uni::logging::Level::DEBUG, slaves[i]->paxos_log->debug_string())
    }

    UNIVERSAL_ASSERT_MESSAGE(
      verify_paxos_logs(slaves),
      "The Paxos Logs should agree with one another.")
  };
}

TestFunction Tests::test2() {
  return [this](
      Constants const& constants,
      std::vector<std::unique_ptr<SlaveTesting>>& slaves,
      std::vector<std::vector<ChannelTesting*>>& all_channels,
      std::vector<ChannelTesting*>& nonempty_channels) {
    int nodes_failed = 0;
    // Send the client message to the first Universal Slave
    auto client_endpoint_id = uni::net::endpoint_id("client", 10000);
    for (int i = 0; i < 300; i++) {
      // Send a client message to some node in the Paxos Group. The node is
      // chosen randomly.
      auto incoming_message = IncomingMessage(client_endpoint_id,
          build_client_request("m" + std::to_string(i)).SerializeAsString());
      slaves[std::rand() % slaves.size()]->scheduler->queue_message(incoming_message);
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
    for (int i = 0; i < 5; i++) {
      LOG(uni::logging::Level::DEBUG, slaves[i]->paxos_log->debug_string())
    }

    UNIVERSAL_ASSERT_MESSAGE(
      verify_paxos_logs(slaves),
      "The Paxos Logs should agree with one another.")
  };
}

TestFunction Tests::test3() {
  return [this](
      Constants const& constants,
      std::vector<std::unique_ptr<SlaveTesting>>& slaves,
      std::vector<std::vector<ChannelTesting*>>& all_channels,
      std::vector<ChannelTesting*>& nonempty_channels) {
    bool passed = true;
    // Wait one heartbeat cycle so that the nodes can send each other a heartbeat
    for (int i = 0; i < constants.heartbeat_period; i++) {
      for (int j = 0; j < slaves.size(); j++) {
        slaves[j]->clock->increment_time(1);
      }
      // Exchange all the messages that need to be exchanged
      while (nonempty_channels.size() > 0) {
        nonempty_channels[0]->deliver_message();
      }
    }
    // All failure detectors should report that all of the slaves are still alive.
    for (auto const& slave : slaves) {
      UNIVERSAL_ASSERT_MESSAGE(
        slave->failure_detector->alive_endpoints().size() == slaves.size(),
        "The Paxos Logs should agree with one another.")
    }
    // Now kill one of the slaves
    mark_node_as_unresponsive(all_channels, 0);
    // Increment the clock on all other slaves an amount such that
    // they will detect the failure
    for (int i = 0; i < constants.heartbeat_failure_threshold * constants.heartbeat_period; i++) {
      for (int j = 1; j < slaves.size(); j++) {
        slaves[j]->clock->increment_time(1);
      }
      // Exchange all the messages that need to be exchanged
      while (nonempty_channels.size() > 0) {
        nonempty_channels[0]->deliver_message();
      }
    }
    for (int i = 1; i < slaves.size(); i++) {
      UNIVERSAL_ASSERT_MESSAGE(
        slaves[i]->failure_detector->alive_endpoints().size() == slaves.size() - 1,
        "All failure detectors should report that one slave is dead.")
    }
  };
}

TestFunction Tests::test4() {
  return [this](
      Constants const& constants,
      std::vector<std::unique_ptr<SlaveTesting>>& slaves,
      std::vector<std::vector<ChannelTesting*>>& all_channels,
      std::vector<ChannelTesting*>& nonempty_channels) {
    auto client_endpoint_id = uni::net::endpoint_id("client", 10000);
    auto client_request_id = 0;
    auto simulate_client_requests = [&](int request_count, int target_slave) {
      // Send the client message to the first Universal Slave
      auto final_request_id = client_request_id + request_count;
      for (; client_request_id < final_request_id; client_request_id++) {
        // Send a client message to some node in the Paxos Group. The node is
        // chosen randomly.
        auto incoming_message = IncomingMessage(client_endpoint_id,
            build_client_request("m" + std::to_string(client_request_id)).SerializeAsString());
        slaves[target_slave]->scheduler->queue_message(incoming_message);
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

    UNIVERSAL_ASSERT_MESSAGE(verify_paxos_logs(slaves), "The Paxos Logs should agree with one another.")
    // None of the PaxosLogs should be equal
    for (int i = 0; i < constants.num_slave_servers; i++) {
      for (int j = i + 1; j < constants.num_slave_servers; j++) {
        UNIVERSAL_ASSERT_MESSAGE(!equals(*slaves[i]->paxos_log, *slaves[j]->paxos_log),
          "Not two PaxosLogs should be equal.")
      }
    }

    // Make sure that at least one heartbeat is sent to ensure a leader is known
    // (plus an extra 5 milliseconds to make sure all messages are sent).
    run_for_milliseconds(slaves, nonempty_channels, constants.heartbeat_period + 5);
    // Make sure one sync message is sent to ensure at least one syncing
    // (plus an extra 5 milliseconds to make sure all messages are sent).
    run_for_milliseconds(slaves, nonempty_channels, constants.log_syncer_period + 5);

    UNIVERSAL_ASSERT_MESSAGE(verify_paxos_logs(slaves), "The Paxos Logs should agree with one another.")
    // All of the PaxosLogs should now be equal
    for (int i = 0; i < constants.num_slave_servers; i++) {
      for (int j = i + 1; j < constants.num_slave_servers; j++) {
        UNIVERSAL_ASSERT_MESSAGE(equals(*slaves[i]->paxos_log, *slaves[j]->paxos_log),
          "Paxos Logs should all be equal.")
      }
    }
    LOG(uni::logging::Level::DEBUG, slaves[0]->kvstore->debug_string());
  };
}

// TODO(pasindu): Get rid of these in due time.
static long request_id = 0;
static long timestamp = 0;

MessageWrapper Tests::build_client_request(std::string message) {
  auto request_message = new proto::client::ClientRequest();
  request_message->set_request_id(++request_id);
  request_message->set_request_type(proto::client::ClientRequest_Type_WRITE);
  request_message->set_key("key");
  request_message->set_value(message);
  request_message->set_timestamp(++timestamp);
  auto client_message = new proto::client::ClientMessage();
  client_message->set_allocated_request(request_message);
  auto message_wrapper = proto::message::MessageWrapper();
  message_wrapper.set_allocated_client_message(client_message);
  return message_wrapper;
}

// Looks at the proposer queues and returns true iff there is a task scheduled in one.
bool Tests::some_proposer_queue_nonempty(
  std::vector<std::unique_ptr<SlaveTesting>>& slaves) {
    for (auto const& slave: slaves) {
      if (!slave->proposer_queue->empty()) {
        return true;
      }
    }
    return false;
}

void Tests::run_until_completion(
  std::vector<std::unique_ptr<SlaveTesting>>& slaves,
  std::vector<ChannelTesting*>& nonempty_channels) {
    while (some_proposer_queue_nonempty(slaves) || nonempty_channels.size() > 0) {
      run_for_milliseconds(slaves, nonempty_channels, 1);
    }
}

void Tests::run_for_milliseconds(
  std::vector<std::unique_ptr<SlaveTesting>>& slaves,
  std::vector<ChannelTesting*>& nonempty_channels,
  int milliseconds) {
    // Since we increase the clocks by 1ms, we assume one message is passed along a
    // channel on average (remember there 2 channels between 2 nodes, one for each direction).
    auto n = slaves.size() * (slaves.size() + 1); // the number of messages to exchange
    for (auto t = 0; t < milliseconds; t++) {
      for (auto const& slave: slaves) {
        if (std::rand() % 100 < 99) {
          // This if statement helps simulate unsynchronized clocks. This is a fairly
          // naive method; it doesn't simulate clocks that have slightly different speeds.
          slave->clock->increment_time(1); 
        }
      }
      auto channels_sent = std::unordered_set<ChannelTesting*>();
      for (auto i = 0; i < n; i++) {
        if (std::rand() % 10 < 9) {
          // This if statement helps prevent the exact same number of
          // of messages being exchanged everytime.
          auto channels_not_sent = std::vector<ChannelTesting*>();
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

bool Tests::verify_paxos_logs(std::vector<std::unique_ptr<SlaveTesting>>& slaves) {
  // To verify the logs, we iterate through each one, adding each entry into a
  // Global Paxos Log. If there is an inconsistency in this process, this means
  // that the Paxos Logs aren't consistent. Otherwise, they are consistent.
  std::unordered_map<uni::paxos::index_t, proto::paxos::PaxosLogEntry const> global_log;
  for (auto const& slave : slaves) {
    for (auto const& [index, entry] : slave->paxos_log->get_log()) {
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

bool Tests::equals(PaxosLog& paxos_log1, PaxosLog& paxos_log2) {
  auto log1 = paxos_log1.get_log();
  auto log2 = paxos_log2.get_log();
  if (log1.size() != log2.size()) {
    return false;
  }
  for (auto const& [index, entry] : log1) {
    auto it = log2.find(index);
    if (it == log2.end() || !MessageDifferencer::Equivalent(it->second, entry)) {
      return false;
    }
  }
  return true;
}

void Tests::mark_node_as_unresponsive(std::vector<std::vector<ChannelTesting*>>& channels, unsigned node) {
  for (int i = 0; i < channels.size(); i++) {
    channels[node][i]->set_connection_state(false);
    channels[i][node]->set_connection_state(false);
  }
}

void Tests::mark_node_as_responsive(std::vector<std::vector<ChannelTesting*>>& channels, unsigned node) {
  for (int i = 0; i < channels.size(); i++) {
    channels[node][i]->set_connection_state(true);
    channels[i][node]->set_connection_state(true);
  }
}

} // testing
} // uni
