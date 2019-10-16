#include <chrono>
#include <exception>
#include <iostream>
#include <memory>
#include <string>
#include <thread>
#include <vector>

#include <boost/asio.hpp>
#include <boost/system/error_code.hpp>

#include <async/impl/AsyncSchedulerImpl.h>
#include <constants/constants.h>
#include <logging/log.h>
#include <net/ConnectionsIn.h>
#include <net/ConnectionsOut.h>
#include <net/endpoint_id.h>
#include <net/IncomingMessage.h>
#include <net/impl/ChannelImpl.h>
#include <proto/master.pb.h>
#include <proto/message.pb.h>
#include <paxos/MultiPaxosHandler.h>
#include <paxos/PaxosLog.h>
#include <paxos/PaxosTypes.h>
#include <paxos/SinglePaxosHandler.h>
#include <slave/ClientConnectionHandler.h>
#include <slave/ClientRequestHandler.h>
#include <slave/IncomingMessageHandler.h>
#include <slave/ServerConnectionHandler.h>
#include <utils.h>

using boost::asio::ip::tcp;

/**
 * Arguments:
 * hostname - hostname this server is running on
 * port - port this server is running on
 */
int main(int argc, char* argv[]) {
  auto hostnames = parse_hostnames(argc, argv);
  auto main_serving_hostname = hostnames[0];

  // Initialize constants
  auto const constants = initialize_constants();
  uni::logging::get_log_level() = uni::logging::Level::INFO;
  LOG(uni::logging::Level::DEBUG, "Starting main server on: " + main_serving_hostname + ":" + std::to_string(constants.slave_port))

  // Initialize io_context for background thread (for managing the network
  // and dispatching requests to the server thread).
  auto background_io_context = boost::asio::io_context();
  auto background_work_guard = boost::asio::make_work_guard(background_io_context);
  auto network_thread = std::thread([&background_io_context](){
    background_io_context.run();
  });

  // Initialize io_context for server thread (for processing requests and
  // dispatching responses to the background thread).
  auto server_io_context = boost::asio::io_context();
  auto server_async_scheduler = uni::async::AsyncSchedulerImpl(server_io_context);

  // Schedule main acceptor
  auto connections_in = uni::net::ConnectionsIn(server_async_scheduler);
  auto connections_out = uni::net::ConnectionsOut(constants);
  auto main_acceptor = tcp::acceptor(background_io_context, tcp::endpoint(tcp::v4(), constants.slave_port));
  auto server_connection_handler = uni::slave::ServerConnectionHandler(constants, connections_in, connections_out, main_acceptor, background_io_context);
  auto resolver = tcp::resolver(background_io_context);

  // Wait for a list of all slave nodes from the master
  server_connection_handler.async_accept();
  auto master_socket = tcp::socket(server_io_context);
  auto master_acceptor = tcp::acceptor(server_io_context, tcp::endpoint(tcp::v4(), constants.master_port));
  // Connect to the master
  master_acceptor.accept(master_socket);
  auto master_channel = uni::net::ChannelImpl(std::move(master_socket));
  // Set callback to execute when the master sends data
  master_channel.set_recieve_callback(
    [&resolver, &constants, &background_io_context, &connections_out](std::string message) {
      auto message_wrapper = proto::message::MessageWrapper();
      message_wrapper.ParseFromString(message);
      auto slave_list = message_wrapper.master_message().request().slave_list();
      // Iterate through each slave hostname
      for (int i = 0; i < slave_list.slave_hostnames_size(); i++) {
        auto hostname = slave_list.slave_hostnames(i);
        auto endpoints = resolver.resolve(hostname, std::to_string(constants.slave_port));
        auto socket = std::make_shared<tcp::socket>(background_io_context);
        // Connect with the other slave.
        boost::asio::async_connect(*socket, endpoints,
            [&connections_out, socket](const boost::system::error_code& ec, const tcp::endpoint& endpoint) {
          if (!ec) {
            LOG(uni::logging::Level::DEBUG, "Sent connection to " + socket->remote_endpoint().address().to_string())
            connections_out.add_channel(std::make_shared<uni::net::ChannelImpl>(std::move(*socket)));
          } else {
            LOG(uni::logging::Level::ERROR, "Error sending connection: " + ec.message())
          }
        });
      }
      return false; // The master channel will only send one message, so stop listening after this.
    });
  master_channel.start_listening();
  server_io_context.run(); // Listen for new messages in the master channel
  server_io_context.restart(); // We must restart before run can be called on the io_context again.

  // Schedule client acceptor
  auto paxos_log = uni::paxos::PaxosLog();
  auto paxos_instance_provider = [&constants, &connections_out, &paxos_log](uni::paxos::index_t index) {
    return uni::paxos::SinglePaxosHandler(constants, connections_out, paxos_log, index);
  };
  auto multipaxos_handler = uni::paxos::MultiPaxosHandler(paxos_log, paxos_instance_provider);
  auto client_acceptor = tcp::acceptor(background_io_context, tcp::endpoint(tcp::v4(), constants.client_port));
  auto client_connection_handler = uni::slave::ClientConnectionHandler(server_async_scheduler, client_acceptor);
  auto client_request_handler = uni::slave::ClientRequestHandler(multipaxos_handler);
  auto incoming_message_handler = uni::slave::IncomingMessageHandler(client_request_handler, multipaxos_handler);
  server_async_scheduler.set_callback([&incoming_message_handler](uni::net::IncomingMessage message){
    incoming_message_handler.handle(message);
  });

  client_connection_handler.async_accept();

  LOG(uni::logging::Level::DEBUG, "Setup finished")
  auto server_work_guard = boost::asio::make_work_guard(server_io_context);
  server_io_context.run();
}
