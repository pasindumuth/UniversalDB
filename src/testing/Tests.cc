#include "Tests.h"

#include <unordered_map>
#include <memory>

#include <async/testing/AsyncSchedulerTesting.h>
#include <net/IncomingMessage.h>
#include <net/testing/ChannelTesting.h>
#include <paxos/PaxosTypes.h>
#include <proto/paxos.pb.h>

namespace uni {
namespace testing {

using proto::message::MessageWrapper;
using uni::async::AsyncSchedulerTesting;
using uni::net::ChannelTesting;
using uni::paxos::PaxosLog;
using uni::net::IncomingMessage;

TestFunction Tests::test1() {
  return [this](
      std::vector<std::unique_ptr<AsyncSchedulerTesting>>& schedulers,
      std::vector<ChannelTesting*>& nonempty_channels,
      std::vector<std::unique_ptr<PaxosLog>>& paxos_logs) {
    std::srand(0);
    // Send the client message to the first Universal Slave
    auto client_endpoint_id = uni::net::endpoint_id("client", 10000);
    for (int i = 0; i < 10; i++) {
      // Create a message that a client would send.
      auto incoming_message = IncomingMessage(client_endpoint_id,
          build_client_request("m" + std::to_string(i)).SerializeAsString());
      schedulers[0]->schedule_async(incoming_message);

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
      paxos_logs[i]->debug_print();
    }

    if (verify_paxos_logs(paxos_logs)) {
      std::cout << "PASSED!!!!!!!!!!!!" << std::endl;
    } else {
      std::cout << "FAILED!!!!!!!!!!!!" << std::endl;
    }
  };
}

TestFunction Tests::test2() {
  return [this](
      std::vector<std::unique_ptr<AsyncSchedulerTesting>>& schedulers,
      std::vector<ChannelTesting*>& nonempty_channels,
      std::vector<std::unique_ptr<PaxosLog>>& paxos_logs) {
    std::srand(0);
    // Send the client message to the first Universal Slave
    auto client_endpoint_id = uni::net::endpoint_id("client", 10000);
    for (int i = 0; i < 10; i++) {
      // Create a message that a client would send.
      auto incoming_message = IncomingMessage(client_endpoint_id,
          build_client_request("m" + std::to_string(i)).SerializeAsString());
      schedulers[0]->schedule_async(incoming_message);

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
      paxos_logs[i]->debug_print();
    }

    if (verify_paxos_logs(paxos_logs)) {
      std::cout << "PASSED!!!!!!!!!!!!" << std::endl;
    } else {
      std::cout << "FAILED!!!!!!!!!!!!" << std::endl;
    }
  };
}

TestFunction Tests::test3() {
  return [this](
      std::vector<std::unique_ptr<AsyncSchedulerTesting>>& schedulers,
      std::vector<ChannelTesting*>& nonempty_channels,
      std::vector<std::unique_ptr<PaxosLog>>& paxos_logs) {
    std::srand(0);
    // Send the client message to the first Universal Slave
    auto client_endpoint_id = uni::net::endpoint_id("client", 10000);
    for (int i = 0; i < 300; i++) {
      // Send a client message to some node in the Paxos Group. The node is
      // chosen randomly.
      auto incoming_message = IncomingMessage(client_endpoint_id,
          build_client_request("m" + std::to_string(i)).SerializeAsString());
      schedulers[std::rand() % schedulers.size()]->schedule_async(incoming_message);

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
      paxos_logs[i]->debug_print();
    }

    if (verify_paxos_logs(paxos_logs)) {
      std::cout << "PASSED!!!!!!!!!!!!" << std::endl;
    } else {
      std::cout << "FAILED!!!!!!!!!!!!" << std::endl;
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

bool Tests::verify_paxos_logs(std::vector<std::unique_ptr<PaxosLog>>& paxos_logs) {
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

  auto ss = std::stringstream();
  ss << "Printing global log:" << std::endl;
  for (auto const& [index, entry] : global_log) {
    ss << "index: " << index << ", entry: " << entry.SerializeAsString() << std::endl;
  }
  ss << "End of PaxosLog" << std::endl;
  std::cout << ss.str() << std::endl;

  return true;
}

} // testing
} // uni
