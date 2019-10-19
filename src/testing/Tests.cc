#include "Tests.h"

#include <unordered_map>
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
    std::srand(0);
    // Send the client message to the first Universal Slave
    auto client_endpoint_id = uni::net::endpoint_id("client", 10000);
    for (int i = 0; i < 10; i++) {
      // Create a message that a client would send.
      auto incoming_message = IncomingMessage(client_endpoint_id,
          build_client_request("m" + std::to_string(i)).SerializeAsString());
      slaves[0]->scheduler->queue_message(incoming_message);

      // Simulate the message exchanging of all Slaves until there are no more messages to send.
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
    for (int i = 0; i < 5; i++) {
      slaves[i]->paxos_log->debug_print();
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
    std::srand(0);
    // Send the client message to the first Universal Slave
    auto client_endpoint_id = uni::net::endpoint_id("client", 10000);
    for (int i = 0; i < 10; i++) {
      // Create a message that a client would send.
      auto incoming_message = IncomingMessage(client_endpoint_id,
          build_client_request("m" + std::to_string(i)).SerializeAsString());
      slaves[0]->scheduler->queue_message(incoming_message);

      // Simulate the message exchanging of all Slaves until there are no more messages to send.
      while (nonempty_channels.size() > 0) {
        // We use modulus to reduce the random number to the range we want.
        // There will be minor bias with this method, but this isn't significant,
        // and so isn't a problem for us.
        int r = std::rand() % nonempty_channels.size();
        auto channel = nonempty_channels[r];
        int should_keep = std::rand() % 4;
        if (should_keep) {
          // simulate a successful delivery of the message
          channel->deliver_message();
        } else {
          // simulate a drop of the message
          channel->drop_message();
        }
      }
    }
    // Now that the simulation is done, print out the Paxos Log and see what we have.
    for (int i = 0; i < 5; i++) {
      slaves[i]->paxos_log->debug_print();
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
    std::srand(0);
    // Send the client message to the first Universal Slave
    auto client_endpoint_id = uni::net::endpoint_id("client", 10000);
    for (int i = 0; i < 300; i++) {
      // Send a client message to some node in the Paxos Group. The node is
      // chosen randomly.
      auto incoming_message = IncomingMessage(client_endpoint_id,
          build_client_request("m" + std::to_string(i)).SerializeAsString());
      slaves[std::rand() % slaves.size()]->scheduler->queue_message(incoming_message);

      // Simulate the message exchanging of all Slaves. There is a 1%
      // chance that we'll stop sending messages and move on.
      while (nonempty_channels.size() > 0 && ((std::rand() % 100) != 0)) {
        // We use modulus to reduce the random number to the range we want.
        // There will be minor bias with this method, but this isn't significant,
        // and so isn't a problem for us.
        int r = std::rand() % nonempty_channels.size();
        auto channel = nonempty_channels[r];
        int should_keep = std::rand() % 4;
        if (should_keep) {
          // simulate a successful delivery of the message
          channel->deliver_message();
        } else {
          // simulate a drop of the message
          channel->drop_message();
        }
      }
    }
    // Now that the simulation is done, print out the Paxos Log and see what we have.
    for (int i = 0; i < 5; i++) {
      slaves[i]->paxos_log->debug_print();
    }

    UNIVERSAL_ASSERT_MESSAGE(
      verify_paxos_logs(slaves),
      "The Paxos Logs should agree with one another.")
  };
}

TestFunction Tests::test4() {
  return [this](
      Constants const& constants,
      std::vector<std::unique_ptr<SlaveTesting>>& slaves,
      std::vector<std::vector<ChannelTesting*>>& all_channels,
      std::vector<ChannelTesting*>& nonempty_channels) {
    std::srand(0);
    int nodes_failed = 0;
    // Send the client message to the first Universal Slave
    auto client_endpoint_id = uni::net::endpoint_id("client", 10000);
    for (int i = 0; i < 300; i++) {
      // Send a client message to some node in the Paxos Group. The node is
      // chosen randomly.
      auto incoming_message = IncomingMessage(client_endpoint_id,
          build_client_request("m" + std::to_string(i)).SerializeAsString());
      slaves[std::rand() % slaves.size()]->scheduler->queue_message(incoming_message);

      // Simulate the message exchanging of all Slaves. There is a 1%
      // chance that we'll stop sending messages and move on.
      while (nonempty_channels.size() > 0 && ((std::rand() % 100) != 0)) {
        // We use modulus to reduce the random number to the range we want.
        // There will be minor bias with this method, but this isn't significant,
        // and so isn't a problem for us.
        int r = std::rand() % nonempty_channels.size();
        auto channel = nonempty_channels[r];
        int should_keep = std::rand() % 4;
        if (should_keep) {
          // simulate a successful delivery of the message
          channel->deliver_message();
        } else {
          // simulate a drop of the message
          channel->drop_message();
        }

        // Fail a node if there isn't already a failed node.
        if (nodes_failed < 3 && (std::rand() % 1000 == 0)) {
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
    }
    // Now that the simulation is done, print out the Paxos Log and see what we have.
    for (int i = 0; i < 5; i++) {
      slaves[i]->paxos_log->debug_print();
    }

    UNIVERSAL_ASSERT_MESSAGE(
      verify_paxos_logs(slaves),
      "The Paxos Logs should agree with one another.")
  };
}

TestFunction Tests::test5() {
  return [this](
      Constants const& constants,
      std::vector<std::unique_ptr<SlaveTesting>>& slaves,
      std::vector<std::vector<ChannelTesting*>>& all_channels,
      std::vector<ChannelTesting*>& nonempty_channels) {
    std::srand(0);
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

TestFunction Tests::test6() {
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

        while (nonempty_channels.size() > 0) {
          nonempty_channels[0]->deliver_message();
        }
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
    for (int i = 0; i < constants.heartbeat_period; i++) {
      for (int j = 0; j < slaves.size(); j++) {
        slaves[j]->clock->increment_time(1);
      }
      // Exchange all the messages that need to be exchanged
      while (nonempty_channels.size() > 0) {
        nonempty_channels[0]->deliver_message();
      }
    }

    // Make sure one sync message is sent to ensure at least one syncing
    for (int i = 0; i < constants.log_syncer_period; i++) {
      for (int j = 0; j < slaves.size(); j++) {
        slaves[j]->clock->increment_time(1);
      }
      // Exchange all the messages that need to be exchanged
      while (nonempty_channels.size() > 0) {
        nonempty_channels[0]->deliver_message();
      }
    }

    UNIVERSAL_ASSERT_MESSAGE(verify_paxos_logs(slaves), "The Paxos Logs should agree with one another.")
    // All of the PaxosLogs should now be equal
    for (int i = 0; i < constants.num_slave_servers; i++) {
      for (int j = i + 1; j < constants.num_slave_servers; j++) {
        UNIVERSAL_ASSERT_MESSAGE(equals(*slaves[i]->paxos_log, *slaves[j]->paxos_log),
          "Paxos Logs should all be equal.")
      }
    }
  };
}

MessageWrapper Tests::build_client_request(std::string message) {
  auto request_message = new proto::client::ClientRequest();
  request_message->set_request_id(0);
  request_message->set_request_type(proto::client::ClientRequest_Type_READ);
  request_message->set_data(message);
  auto client_message = new proto::client::ClientMessage();
  client_message->set_allocated_request(request_message);
  auto message_wrapper = proto::message::MessageWrapper();
  message_wrapper.set_allocated_client_message(client_message);
  return message_wrapper;
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
