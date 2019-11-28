#include <string>
#include <iostream>

#include <boost/asio.hpp>
#include <boost/system/error_code.hpp>

#include <logging/log.h>
#include <net/impl/ChannelImpl.h>
#include <proto/client.pb.h>
#include <proto/message.pb.h>
#include <utils.h>

using boost::asio::ip::tcp;

int main(int argc, char* argv[]) {
  auto hostnames = parse_hostnames(argc, argv);
  auto hostname = hostnames[0];

  auto const constants = initialize_constants();
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

  for (int request_id = 0;; request_id++) {
    char message_array[50];
    std::cin >> message_array;
    std::string message(message_array);

    auto request_message = new proto::client::ClientRequest();
    request_message->set_request_id(request_id);
    request_message->set_request_type(proto::client::ClientRequest_Type_READ);
    request_message->set_value(message);

    auto client_message = new proto::client::ClientMessage();
    client_message->set_allocated_request(request_message);

    auto message_wrapper = new proto::message::MessageWrapper();
    message_wrapper->set_allocated_client_message(client_message);

    channel.queue_send(message_wrapper->SerializeAsString());
  }
}
