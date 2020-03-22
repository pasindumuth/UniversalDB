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
#include <proto/client.pb.h>
#include <proto/message.pb.h>
#include <utils.h>
#include <utils/pbutil.h>

using boost::asio::ip::tcp;

std::vector<std::string> parse_input(std::string&& message) {
  auto parsed_output = std::vector<std::string>();
  auto delimiter = std::string(", ");
  auto pos = 0;
  while ((pos = message.find(delimiter)) != std::string::npos) {
      auto token = message.substr(0, pos);
      parsed_output.push_back(token);
      message.erase(0, pos + delimiter.length());
  }
  parsed_output.push_back(std::string(message));
  return parsed_output;
}

int main(int argc, char* argv[]) {
  auto hostnames = parse_hostnames(argc, argv);
  auto hostname = hostnames[0];

  auto const constants = initialize_constants();
  uni::logging::get_log_level() = uni::logging::Level::INFO;
  LOG(uni::logging::Level::INFO, "Starting client on: " + hostname + ":" + std::to_string(constants.client_port))

  boost::asio::io_context io_context;
  auto work = boost::asio::make_work_guard(io_context);
  std::thread network_thread([&io_context](){
    io_context.run();
  });

  tcp::resolver resolver(io_context);
  tcp::resolver::results_type endpoints = resolver.resolve(hostname, std::to_string(constants.client_port));

  tcp::socket socket(io_context);
  boost::asio::connect(socket, endpoints);
  uni::net::ChannelImpl channel(std::move(socket));
  channel.add_receive_callback([](std::string serialized_message) {
    auto message_wrapper = proto::message::MessageWrapper();
    message_wrapper.ParseFromString(serialized_message);
    LOG(uni::logging::Level::INFO, message_wrapper.DebugString());
    return true;
  });
  channel.start_listening();

  for (auto request_id = 0;; request_id++) {
    auto message = std::string();
    std::getline(std::cin, message); // Example input: d, t, k, v
    auto parsed_output = parse_input(std::move(message));

    UNIVERSAL_ASSERT_MESSAGE(parsed_output.size() >= 4,
      "The input must contain 4 elements, (database_id, table_id, key, value), delimited by a ', '.");

    auto message_wrapper = proto::message::MessageWrapper();
    auto client_message = new proto::client::ClientMessage();
    auto request_message = new proto::client::ClientRequest();
    request_message->set_request_id(std::to_string(request_id));
    request_message->set_request_type(proto::client::ClientRequest::WRITE);
    request_message->set_allocated_database_id(uni::utils::pb::string(parsed_output[0]));
    request_message->set_allocated_table_id(uni::utils::pb::string(parsed_output[1]));
    request_message->set_allocated_key(uni::utils::pb::string(parsed_output[2]));
    request_message->set_allocated_value(uni::utils::pb::string(parsed_output[3]));
    request_message->set_allocated_timestamp(uni::utils::pb::uint64(std::time(nullptr)));
    client_message->set_allocated_request(request_message);
    message_wrapper.set_allocated_client_message(client_message);
    channel.queue_send(message_wrapper.SerializeAsString());
  }
}
