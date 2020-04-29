#include <ctime>
#include <iostream>
#include <string>
#include <vector>

#include <boost/asio.hpp>
#include <boost/system/error_code.hpp>
#include <google/protobuf/wrappers.pb.h>

#include <assert/assert.h>
#include <common/common.h>
#include <net/impl/ChannelImpl.h>
#include <proto/message_client.pb.h>
#include <proto/message.pb.h>
#include <utils.h>
#include <utils/pbutil.h>

using boost::asio::ip::tcp;

std::vector<std::string> parse_input(std::string&& message) {
  auto parsed_output = std::vector<std::string>();
  auto delimiter = std::string(" ");
  auto pos = 0;
  while ((pos = message.find(delimiter)) != std::string::npos) {
      auto token = message.substr(0, pos);
      parsed_output.push_back(token);
      message.erase(0, pos + delimiter.length());
  }
  parsed_output.push_back(std::string(message));
  return parsed_output;
}

// Takes in the slave_hostname and the master_hostname
int main(int argc, char* argv[]) {
  auto hostnames = parse_hostnames(argc, argv);
  auto slave_hostname = hostnames[0];
  auto master_hostname = hostnames[1];

  auto const constants = initialize_constants();
  uni::logging::get_log_level() = uni::logging::Level::INFO;
  LOG(uni::logging::Level::INFO, "Starting client. Connecting to slave: '" + slave_hostname + "', and master: '" + master_hostname + "'.")

  auto io_context = boost::asio::io_context();
  auto work = boost::asio::make_work_guard(io_context);
  std::thread network_thread([&io_context](){
    io_context.run();
  });

  auto resolver = tcp::resolver(io_context);

  auto slave_endpoints = tcp::resolver::results_type(resolver.resolve(slave_hostname, std::to_string(constants.client_port)));
  auto socket = tcp::socket(io_context);
  boost::asio::connect(socket, slave_endpoints);
  auto slave_channel = uni::net::ChannelImpl(std::move(socket));
  slave_channel.add_receive_callback([](std::string serialized_message) {
    auto message_wrapper = proto::message::MessageWrapper();
    message_wrapper.ParseFromString(serialized_message);
    LOG(uni::logging::Level::INFO, message_wrapper.DebugString());
    return true;
  });

  slave_channel.start_listening();

  auto master_endpoints = tcp::resolver::results_type(resolver.resolve(master_hostname, std::to_string(constants.client_port)));
  auto socket2 = tcp::socket(io_context);
  boost::asio::connect(socket2, master_endpoints);
  auto master_channel = uni::net::ChannelImpl(std::move(socket2));
  master_channel.add_receive_callback([](std::string serialized_message) {
    auto message_wrapper = proto::message::MessageWrapper();
    message_wrapper.ParseFromString(serialized_message);
    LOG(uni::logging::Level::INFO, message_wrapper.DebugString());
    return true;
  });

  master_channel.start_listening();

  for (auto request_id = 0;; request_id++) {
    auto message = std::string();
    std::getline(std::cin, message);
    auto parsed_output = parse_input(std::move(message));

    if (parsed_output.size() < 1) {
      LOG(uni::logging::Level::INFO, "Command name missing")
    }
    auto const& command = parsed_output[0];
    if (command == "find") { // Example input: find d t k
      if (parsed_output.size() < 4) {
        LOG(uni::logging::Level::INFO, "Missing argument(s): 'database_id' 'table_id' 'key'")
      }
      auto message_wrapper = proto::message::MessageWrapper();
      auto client_message = new proto::message::client::ClientMessage();
      auto find_key_request = new proto::message::client::FindKeyRangeRequest();
      find_key_request->set_database_id(parsed_output[1]);
      find_key_request->set_table_id(parsed_output[2]);
      find_key_request->set_key(parsed_output[3]);
      client_message->set_allocated_find_key_range_request(find_key_request);
      message_wrapper.set_allocated_client_message(client_message);
      master_channel.queue_send(message_wrapper.SerializeAsString());
    } else if (command == "insert") { // Example input: insert d t k v
      if (parsed_output.size() < 5) {
        LOG(uni::logging::Level::INFO, "Missing arguments(s): 'database_id' 'table_id' 'key' 'value'")
      }
      auto message_wrapper = proto::message::MessageWrapper();
      auto client_message = new proto::message::client::ClientMessage();
      auto request_message = new proto::message::client::ClientRequest();
      request_message->set_request_id(std::to_string(request_id));
      request_message->set_request_type(proto::message::client::ClientRequest::WRITE);
      request_message->set_database_id(parsed_output[1]);
      request_message->set_table_id(parsed_output[2]);
      request_message->set_key(parsed_output[3]);
      request_message->set_allocated_value(uni::utils::pb::string(parsed_output[4]));
      request_message->set_allocated_timestamp(uni::utils::pb::uint64(std::time(nullptr)));
      client_message->set_allocated_request(request_message);
      message_wrapper.set_allocated_client_message(client_message);
      slave_channel.queue_send(message_wrapper.SerializeAsString());
    }
  }
}
