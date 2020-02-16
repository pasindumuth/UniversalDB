#include <ctime>
#include <iostream>
#include <string>

#include <boost/asio.hpp>
#include <boost/system/error_code.hpp>
#include <google/protobuf/wrappers.pb.h>

#include <common/common.h>
#include <net/impl/ChannelImpl.h>
#include <proto/client.pb.h>
#include <proto/message.pb.h>
#include <utils.h>
#include <utils/pbutil.h>

using boost::asio::ip::tcp;

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
    char message_array[50];
    std::cin >> message_array;
    std::string message(message_array);
    std::string key = message.substr(0, 2);
    std::string value = message.substr(2, 2);

    auto request_message = new proto::client::ClientRequest();
    request_message->set_request_id(std::to_string(request_id));
    request_message->set_request_type(proto::client::ClientRequest::WRITE);
    request_message->set_allocated_key(uni::utils::pb::string(key));
    request_message->set_allocated_value(uni::utils::pb::string(value));
    request_message->set_allocated_timestamp(uni::utils::pb::uint64(std::time(nullptr)));

    auto client_message = new proto::client::ClientMessage();
    client_message->set_allocated_request(request_message);

    auto message_wrapper = new proto::message::MessageWrapper();
    message_wrapper->set_allocated_client_message(client_message);

    channel.queue_send(message_wrapper->SerializeAsString());
  }
}
